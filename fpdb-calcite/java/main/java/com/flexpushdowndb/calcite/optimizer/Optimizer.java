package com.flexpushdowndb.calcite.optimizer;

import com.flexpushdowndb.calcite.rule.EnhancedFilterJoinRule;
import com.flexpushdowndb.calcite.rule.JoinSmallLeftRule;
import com.flexpushdowndb.calcite.schema.SchemaImpl;
import com.flexpushdowndb.calcite.schema.SchemaReader;
import com.google.common.collect.ImmutableList;
import org.apache.calcite.adapter.enumerable.EnumerableConvention;
import org.apache.calcite.adapter.enumerable.EnumerableRules;
import org.apache.calcite.config.CalciteConnectionConfig;
import org.apache.calcite.config.CalciteConnectionProperty;
import org.apache.calcite.jdbc.CalciteSchema;
import org.apache.calcite.jdbc.JavaTypeFactoryImpl;
import org.apache.calcite.plan.*;
import org.apache.calcite.plan.hep.HepPlanner;
import org.apache.calcite.plan.hep.HepProgram;
import org.apache.calcite.plan.hep.HepProgramBuilder;
import org.apache.calcite.plan.volcano.VolcanoPlanner;
import org.apache.calcite.prepare.CalciteCatalogReader;
import org.apache.calcite.rel.RelNode;
import org.apache.calcite.rel.metadata.CachingRelMetadataProvider;
import org.apache.calcite.rel.metadata.ChainedRelMetadataProvider;
import org.apache.calcite.rel.metadata.JaninoRelMetadataProvider;
import org.apache.calcite.rel.metadata.RelMetadataProvider;
import org.apache.calcite.rel.rules.CoreRules;
import org.apache.calcite.rel.type.RelDataTypeFactory;
import org.apache.calcite.rex.RexBuilder;
import org.apache.calcite.sql.SqlNode;
import org.apache.calcite.sql.fun.SqlStdOperatorTable;
import org.apache.calcite.sql.parser.SqlParser;
import org.apache.calcite.sql.validate.SqlValidator;
import org.apache.calcite.sql.validate.SqlValidatorUtil;
import org.apache.calcite.sql2rel.RelDecorrelator;
import org.apache.calcite.sql2rel.RelFieldTrimmer;
import org.apache.calcite.sql2rel.SqlToRelConverter;
import org.apache.calcite.sql2rel.StandardConvertletTable;
import org.apache.calcite.tools.*;

import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Optimizer {
  private static final RelOptTable.ViewExpander NOOP_EXPANDER = (type, query, schema, path) -> null;
  private final Path resourcePath;
  private final RelDataTypeFactory typeFactory;
  private final RelOptCluster cluster;
  private final RelOptPlanner planner;
  private final RelBuilder relBuilder;
  private final CalciteSchema rootSchema;

  // assigned during query parsing
  private SqlValidator sqlValidator = null;

  public Optimizer(Path resourcePath) {
    // Initialize
    this.resourcePath = resourcePath;
    this.typeFactory = new JavaTypeFactoryImpl();
    this.cluster = newCluster(typeFactory);
    this.planner = cluster.getPlanner();
    this.relBuilder = RelBuilder.proto(planner.getContext()).create(cluster, null);
    this.rootSchema = CalciteSchema.createRootSchema(false, true);

    // Set RelMetadataProvider, using DefaultRelMetadataProvider
    List<RelMetadataProvider> metadataProviders = new ArrayList<>();
    metadataProviders.add(JaninoRelMetadataProvider.DEFAULT);
    this.planner.registerMetadataProviders(metadataProviders);
    RelMetadataProvider chainedMetadataProvider = ChainedRelMetadataProvider.of(metadataProviders);
    CachingRelMetadataProvider cachingMetadataProvider = new CachingRelMetadataProvider(chainedMetadataProvider, planner);
    this.cluster.setMetadataProvider(cachingMetadataProvider);
  }

  public RelNode planQuery(String query, String schemaName) throws Exception {
    // Parse and Validate
    RelNode logicalPlan = parseAndValidate(query, schemaName);

    // Decorrelate
    RelNode decorrelatedPlan = decorrelate(logicalPlan);

    // Pre-join filter pushdown
    RelNode preFilterPushdownPlan = filterPushdown(decorrelatedPlan);

    // Join optimization
    RelNode joinOptPlan = joinOptimize(preFilterPushdownPlan);

    // Post-join filter pushdown
    RelNode postFilterPushdownPlan = filterPushdown(joinOptPlan);

    // Trim unused fields
    RelNode trimmedPhysicalPlan = trim(postFilterPushdownPlan);

    // Heuristics to apply
    return postHeuristics(trimmedPhysicalPlan);
  }

  private RelNode parseAndValidate(String query, String schemaName) throws Exception {
    // Load schema if not loaded
    if (!rootSchema.getSubSchemaMap().containsKey(schemaName)) {
      SchemaImpl schema = SchemaReader.readSchema(resourcePath, schemaName);
      rootSchema.add(schemaName, schema);
    }

    // Create an SQL parser and parse the query into AST
    SqlParser parser = SqlParser.create(query);
    SqlNode parseAst = parser.parseQuery();

    // Create a CatalogReader and an SQL validator to validate the AST
    CalciteCatalogReader catalogReader = new CalciteCatalogReader(rootSchema, Collections.singletonList(schemaName),
            typeFactory, CalciteConnectionConfig.DEFAULT.set(CalciteConnectionProperty.CASE_SENSITIVE, "false"));
    sqlValidator = SqlValidatorUtil.newValidator(SqlStdOperatorTable.instance(), catalogReader,
            typeFactory, SqlValidator.Config.DEFAULT);
    SqlNode validAst = sqlValidator.validate(parseAst);

    // Convert the AST into a RelNode
    SqlToRelConverter sqlToRelConverter = new SqlToRelConverter(NOOP_EXPANDER,
            sqlValidator,
            catalogReader,
            cluster,
            StandardConvertletTable.INSTANCE,
            SqlToRelConverter.config());
    return sqlToRelConverter.convertQuery(validAst, false, true).rel;
  }

  private RelNode decorrelate(RelNode relNode) {
    // Need to push filter past correlate first
    HepProgram hepProgram = new HepProgramBuilder()
            .addRuleCollection(ImmutableList.of(
                    CoreRules.FILTER_PROJECT_TRANSPOSE,
                    CoreRules.FILTER_AGGREGATE_TRANSPOSE,
                    CoreRules.FILTER_CORRELATE))
            .build();
    HepPlanner hepPlanner = new HepPlanner(hepProgram);
    hepPlanner.setRoot(relNode);
    RelNode pushFilterPastCorrelatePlan = hepPlanner.findBestExp();
    return RelDecorrelator.decorrelateQuery(pushFilterPastCorrelatePlan, relBuilder);
  }

  private RelNode filterPushdown(RelNode relNode) {
    HepProgram hepProgram = new HepProgramBuilder()
            .addRuleCollection(ImmutableList.of(
                    CoreRules.FILTER_PROJECT_TRANSPOSE,
                    CoreRules.FILTER_AGGREGATE_TRANSPOSE,
                    EnhancedFilterJoinRule.WITH_FILTER,
                    EnhancedFilterJoinRule.NO_FILTER))
            .build();
    HepPlanner hepPlanner = new HepPlanner(hepProgram);
    hepPlanner.setRoot(relNode);
    return hepPlanner.findBestExp();
  }

  private RelNode joinOptimize(RelNode relNode) {
    // join optimize
    Program program = Programs.heuristicJoinOrder(getJoinOptimizeRules(), false, 0);
    return program.run(
            planner,
            relNode,
            relNode.getTraitSet().plus(EnumerableConvention.INSTANCE),
            Collections.emptyList(),
            Collections.emptyList()
    );
  }

  private RelNode trim(RelNode relNode) {
    RelFieldTrimmer trimmer = new RelFieldTrimmer(sqlValidator, relBuilder);
    RelNode trimmedPlan = trimmer.trim(relNode);

    // Convert trimmedPlan to physical
    Program program = Programs.of(RuleSets.ofList(getConvertToPhysicalRules()));
    return program.run(
            planner,
            trimmedPlan,
            trimmedPlan.getTraitSet().plus(EnumerableConvention.INSTANCE),
            Collections.emptyList(),
            Collections.emptyList()
    );
  }

  private RelNode postHeuristics(RelNode relNode) {
    HepProgram hepProgram = new HepProgramBuilder()
            .addRuleCollection(ImmutableList.of(
                    CoreRules.PROJECT_MERGE,
                    CoreRules.PROJECT_FILTER_TRANSPOSE,
                    CoreRules.PROJECT_REMOVE,
                    JoinSmallLeftRule.INSTANCE,
                    EnumerableRules.ENUMERABLE_PROJECT_RULE))
            .build();
    HepPlanner hepPlanner = new HepPlanner(hepProgram);
    hepPlanner.setRoot(relNode);
    return hepPlanner.findBestExp();
  }

  private static List<RelOptRule> getJoinOptimizeRules() {
    List<RelOptRule> ruleList = getConvertToPhysicalRules();
    ruleList.add(CoreRules.PROJECT_MERGE);
    ruleList.add(CoreRules.PROJECT_AGGREGATE_MERGE);
    ruleList.add(CoreRules.FILTER_AGGREGATE_TRANSPOSE);
    ruleList.add(CoreRules.JOIN_TO_SEMI_JOIN);
    ruleList.add(CoreRules.PROJECT_TO_SEMI_JOIN);
    return ruleList;
  }

  private static List<RelOptRule> getConvertToPhysicalRules() {
    List<RelOptRule> ruleList = new ArrayList<>();
    ruleList.add(EnumerableRules.ENUMERABLE_TABLE_SCAN_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_FILTER_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_PROJECT_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_JOIN_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_AGGREGATE_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_SORT_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_LIMIT_SORT_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_LIMIT_RULE);
    ruleList.add(EnumerableRules.ENUMERABLE_SORTED_AGGREGATE_RULE);
    return ruleList;
  }

  private static RelOptCluster newCluster(RelDataTypeFactory typeFactory) {
    RelOptPlanner planner = new VolcanoPlanner(RelOptCostImpl.FACTORY, Contexts.of(CalciteConnectionConfig.DEFAULT));
    planner.addRelTraitDef(ConventionTraitDef.INSTANCE);
    return RelOptCluster.create(planner, new RexBuilder(typeFactory));
  }
}
