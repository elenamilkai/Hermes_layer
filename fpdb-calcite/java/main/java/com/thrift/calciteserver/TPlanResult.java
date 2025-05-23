/**
 * Autogenerated by Thrift Compiler (0.17.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
package com.thrift.calciteserver;

@SuppressWarnings({"cast", "rawtypes", "serial", "unchecked", "unused"})
@javax.annotation.Generated(value = "Autogenerated by Thrift Compiler (0.17.0)", date = "2024-11-19")
public class TPlanResult implements org.apache.thrift.TBase<TPlanResult, TPlanResult._Fields>, java.io.Serializable, Cloneable, Comparable<TPlanResult> {
  private static final org.apache.thrift.protocol.TStruct STRUCT_DESC = new org.apache.thrift.protocol.TStruct("TPlanResult");

  private static final org.apache.thrift.protocol.TField PLAN_RESULT_FIELD_DESC = new org.apache.thrift.protocol.TField("plan_result", org.apache.thrift.protocol.TType.STRING, (short)1);
  private static final org.apache.thrift.protocol.TField EXECUTION_TIME_MS_FIELD_DESC = new org.apache.thrift.protocol.TField("execution_time_ms", org.apache.thrift.protocol.TType.I64, (short)2);

  private static final org.apache.thrift.scheme.SchemeFactory STANDARD_SCHEME_FACTORY = new TPlanResultStandardSchemeFactory();
  private static final org.apache.thrift.scheme.SchemeFactory TUPLE_SCHEME_FACTORY = new TPlanResultTupleSchemeFactory();

  public @org.apache.thrift.annotation.Nullable java.lang.String plan_result; // required
  public long execution_time_ms; // required

  /** The set of fields this struct contains, along with convenience methods for finding and manipulating them. */
  public enum _Fields implements org.apache.thrift.TFieldIdEnum {
    PLAN_RESULT((short)1, "plan_result"),
    EXECUTION_TIME_MS((short)2, "execution_time_ms");

    private static final java.util.Map<java.lang.String, _Fields> byName = new java.util.HashMap<java.lang.String, _Fields>();

    static {
      for (_Fields field : java.util.EnumSet.allOf(_Fields.class)) {
        byName.put(field.getFieldName(), field);
      }
    }

    /**
     * Find the _Fields constant that matches fieldId, or null if its not found.
     */
    @org.apache.thrift.annotation.Nullable
    public static _Fields findByThriftId(int fieldId) {
      switch(fieldId) {
        case 1: // PLAN_RESULT
          return PLAN_RESULT;
        case 2: // EXECUTION_TIME_MS
          return EXECUTION_TIME_MS;
        default:
          return null;
      }
    }

    /**
     * Find the _Fields constant that matches fieldId, throwing an exception
     * if it is not found.
     */
    public static _Fields findByThriftIdOrThrow(int fieldId) {
      _Fields fields = findByThriftId(fieldId);
      if (fields == null) throw new java.lang.IllegalArgumentException("Field " + fieldId + " doesn't exist!");
      return fields;
    }

    /**
     * Find the _Fields constant that matches name, or null if its not found.
     */
    @org.apache.thrift.annotation.Nullable
    public static _Fields findByName(java.lang.String name) {
      return byName.get(name);
    }

    private final short _thriftId;
    private final java.lang.String _fieldName;

    _Fields(short thriftId, java.lang.String fieldName) {
      _thriftId = thriftId;
      _fieldName = fieldName;
    }

    @Override
    public short getThriftFieldId() {
      return _thriftId;
    }

    @Override
    public java.lang.String getFieldName() {
      return _fieldName;
    }
  }

  // isset id assignments
  private static final int __EXECUTION_TIME_MS_ISSET_ID = 0;
  private byte __isset_bitfield = 0;
  public static final java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> metaDataMap;
  static {
    java.util.Map<_Fields, org.apache.thrift.meta_data.FieldMetaData> tmpMap = new java.util.EnumMap<_Fields, org.apache.thrift.meta_data.FieldMetaData>(_Fields.class);
    tmpMap.put(_Fields.PLAN_RESULT, new org.apache.thrift.meta_data.FieldMetaData("plan_result", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.STRING)));
    tmpMap.put(_Fields.EXECUTION_TIME_MS, new org.apache.thrift.meta_data.FieldMetaData("execution_time_ms", org.apache.thrift.TFieldRequirementType.DEFAULT, 
        new org.apache.thrift.meta_data.FieldValueMetaData(org.apache.thrift.protocol.TType.I64)));
    metaDataMap = java.util.Collections.unmodifiableMap(tmpMap);
    org.apache.thrift.meta_data.FieldMetaData.addStructMetaDataMap(TPlanResult.class, metaDataMap);
  }

  public TPlanResult() {
  }

  public TPlanResult(
    java.lang.String plan_result,
    long execution_time_ms)
  {
    this();
    this.plan_result = plan_result;
    this.execution_time_ms = execution_time_ms;
    setExecution_time_msIsSet(true);
  }

  /**
   * Performs a deep copy on <i>other</i>.
   */
  public TPlanResult(TPlanResult other) {
    __isset_bitfield = other.__isset_bitfield;
    if (other.isSetPlan_result()) {
      this.plan_result = other.plan_result;
    }
    this.execution_time_ms = other.execution_time_ms;
  }

  @Override
  public TPlanResult deepCopy() {
    return new TPlanResult(this);
  }

  @Override
  public void clear() {
    this.plan_result = null;
    setExecution_time_msIsSet(false);
    this.execution_time_ms = 0;
  }

  @org.apache.thrift.annotation.Nullable
  public java.lang.String getPlan_result() {
    return this.plan_result;
  }

  public TPlanResult setPlan_result(@org.apache.thrift.annotation.Nullable java.lang.String plan_result) {
    this.plan_result = plan_result;
    return this;
  }

  public void unsetPlan_result() {
    this.plan_result = null;
  }

  /** Returns true if field plan_result is set (has been assigned a value) and false otherwise */
  public boolean isSetPlan_result() {
    return this.plan_result != null;
  }

  public void setPlan_resultIsSet(boolean value) {
    if (!value) {
      this.plan_result = null;
    }
  }

  public long getExecution_time_ms() {
    return this.execution_time_ms;
  }

  public TPlanResult setExecution_time_ms(long execution_time_ms) {
    this.execution_time_ms = execution_time_ms;
    setExecution_time_msIsSet(true);
    return this;
  }

  public void unsetExecution_time_ms() {
    __isset_bitfield = org.apache.thrift.EncodingUtils.clearBit(__isset_bitfield, __EXECUTION_TIME_MS_ISSET_ID);
  }

  /** Returns true if field execution_time_ms is set (has been assigned a value) and false otherwise */
  public boolean isSetExecution_time_ms() {
    return org.apache.thrift.EncodingUtils.testBit(__isset_bitfield, __EXECUTION_TIME_MS_ISSET_ID);
  }

  public void setExecution_time_msIsSet(boolean value) {
    __isset_bitfield = org.apache.thrift.EncodingUtils.setBit(__isset_bitfield, __EXECUTION_TIME_MS_ISSET_ID, value);
  }

  @Override
  public void setFieldValue(_Fields field, @org.apache.thrift.annotation.Nullable java.lang.Object value) {
    switch (field) {
    case PLAN_RESULT:
      if (value == null) {
        unsetPlan_result();
      } else {
        setPlan_result((java.lang.String)value);
      }
      break;

    case EXECUTION_TIME_MS:
      if (value == null) {
        unsetExecution_time_ms();
      } else {
        setExecution_time_ms((java.lang.Long)value);
      }
      break;

    }
  }

  @org.apache.thrift.annotation.Nullable
  @Override
  public java.lang.Object getFieldValue(_Fields field) {
    switch (field) {
    case PLAN_RESULT:
      return getPlan_result();

    case EXECUTION_TIME_MS:
      return getExecution_time_ms();

    }
    throw new java.lang.IllegalStateException();
  }

  /** Returns true if field corresponding to fieldID is set (has been assigned a value) and false otherwise */
  @Override
  public boolean isSet(_Fields field) {
    if (field == null) {
      throw new java.lang.IllegalArgumentException();
    }

    switch (field) {
    case PLAN_RESULT:
      return isSetPlan_result();
    case EXECUTION_TIME_MS:
      return isSetExecution_time_ms();
    }
    throw new java.lang.IllegalStateException();
  }

  @Override
  public boolean equals(java.lang.Object that) {
    if (that instanceof TPlanResult)
      return this.equals((TPlanResult)that);
    return false;
  }

  public boolean equals(TPlanResult that) {
    if (that == null)
      return false;
    if (this == that)
      return true;

    boolean this_present_plan_result = true && this.isSetPlan_result();
    boolean that_present_plan_result = true && that.isSetPlan_result();
    if (this_present_plan_result || that_present_plan_result) {
      if (!(this_present_plan_result && that_present_plan_result))
        return false;
      if (!this.plan_result.equals(that.plan_result))
        return false;
    }

    boolean this_present_execution_time_ms = true;
    boolean that_present_execution_time_ms = true;
    if (this_present_execution_time_ms || that_present_execution_time_ms) {
      if (!(this_present_execution_time_ms && that_present_execution_time_ms))
        return false;
      if (this.execution_time_ms != that.execution_time_ms)
        return false;
    }

    return true;
  }

  @Override
  public int hashCode() {
    int hashCode = 1;

    hashCode = hashCode * 8191 + ((isSetPlan_result()) ? 131071 : 524287);
    if (isSetPlan_result())
      hashCode = hashCode * 8191 + plan_result.hashCode();

    hashCode = hashCode * 8191 + org.apache.thrift.TBaseHelper.hashCode(execution_time_ms);

    return hashCode;
  }

  @Override
  public int compareTo(TPlanResult other) {
    if (!getClass().equals(other.getClass())) {
      return getClass().getName().compareTo(other.getClass().getName());
    }

    int lastComparison = 0;

    lastComparison = java.lang.Boolean.compare(isSetPlan_result(), other.isSetPlan_result());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetPlan_result()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.plan_result, other.plan_result);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    lastComparison = java.lang.Boolean.compare(isSetExecution_time_ms(), other.isSetExecution_time_ms());
    if (lastComparison != 0) {
      return lastComparison;
    }
    if (isSetExecution_time_ms()) {
      lastComparison = org.apache.thrift.TBaseHelper.compareTo(this.execution_time_ms, other.execution_time_ms);
      if (lastComparison != 0) {
        return lastComparison;
      }
    }
    return 0;
  }

  @org.apache.thrift.annotation.Nullable
  @Override
  public _Fields fieldForId(int fieldId) {
    return _Fields.findByThriftId(fieldId);
  }

  @Override
  public void read(org.apache.thrift.protocol.TProtocol iprot) throws org.apache.thrift.TException {
    scheme(iprot).read(iprot, this);
  }

  @Override
  public void write(org.apache.thrift.protocol.TProtocol oprot) throws org.apache.thrift.TException {
    scheme(oprot).write(oprot, this);
  }

  @Override
  public java.lang.String toString() {
    java.lang.StringBuilder sb = new java.lang.StringBuilder("TPlanResult(");
    boolean first = true;

    sb.append("plan_result:");
    if (this.plan_result == null) {
      sb.append("null");
    } else {
      sb.append(this.plan_result);
    }
    first = false;
    if (!first) sb.append(", ");
    sb.append("execution_time_ms:");
    sb.append(this.execution_time_ms);
    first = false;
    sb.append(")");
    return sb.toString();
  }

  public void validate() throws org.apache.thrift.TException {
    // check for required fields
    // check for sub-struct validity
  }

  private void writeObject(java.io.ObjectOutputStream out) throws java.io.IOException {
    try {
      write(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(out)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private void readObject(java.io.ObjectInputStream in) throws java.io.IOException, java.lang.ClassNotFoundException {
    try {
      // it doesn't seem like you should have to do this, but java serialization is wacky, and doesn't call the default constructor.
      __isset_bitfield = 0;
      read(new org.apache.thrift.protocol.TCompactProtocol(new org.apache.thrift.transport.TIOStreamTransport(in)));
    } catch (org.apache.thrift.TException te) {
      throw new java.io.IOException(te);
    }
  }

  private static class TPlanResultStandardSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    @Override
    public TPlanResultStandardScheme getScheme() {
      return new TPlanResultStandardScheme();
    }
  }

  private static class TPlanResultStandardScheme extends org.apache.thrift.scheme.StandardScheme<TPlanResult> {

    @Override
    public void read(org.apache.thrift.protocol.TProtocol iprot, TPlanResult struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TField schemeField;
      iprot.readStructBegin();
      while (true)
      {
        schemeField = iprot.readFieldBegin();
        if (schemeField.type == org.apache.thrift.protocol.TType.STOP) { 
          break;
        }
        switch (schemeField.id) {
          case 1: // PLAN_RESULT
            if (schemeField.type == org.apache.thrift.protocol.TType.STRING) {
              struct.plan_result = iprot.readString();
              struct.setPlan_resultIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          case 2: // EXECUTION_TIME_MS
            if (schemeField.type == org.apache.thrift.protocol.TType.I64) {
              struct.execution_time_ms = iprot.readI64();
              struct.setExecution_time_msIsSet(true);
            } else { 
              org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
            }
            break;
          default:
            org.apache.thrift.protocol.TProtocolUtil.skip(iprot, schemeField.type);
        }
        iprot.readFieldEnd();
      }
      iprot.readStructEnd();

      // check for required fields of primitive type, which can't be checked in the validate method
      struct.validate();
    }

    @Override
    public void write(org.apache.thrift.protocol.TProtocol oprot, TPlanResult struct) throws org.apache.thrift.TException {
      struct.validate();

      oprot.writeStructBegin(STRUCT_DESC);
      if (struct.plan_result != null) {
        oprot.writeFieldBegin(PLAN_RESULT_FIELD_DESC);
        oprot.writeString(struct.plan_result);
        oprot.writeFieldEnd();
      }
      oprot.writeFieldBegin(EXECUTION_TIME_MS_FIELD_DESC);
      oprot.writeI64(struct.execution_time_ms);
      oprot.writeFieldEnd();
      oprot.writeFieldStop();
      oprot.writeStructEnd();
    }

  }

  private static class TPlanResultTupleSchemeFactory implements org.apache.thrift.scheme.SchemeFactory {
    @Override
    public TPlanResultTupleScheme getScheme() {
      return new TPlanResultTupleScheme();
    }
  }

  private static class TPlanResultTupleScheme extends org.apache.thrift.scheme.TupleScheme<TPlanResult> {

    @Override
    public void write(org.apache.thrift.protocol.TProtocol prot, TPlanResult struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol oprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet optionals = new java.util.BitSet();
      if (struct.isSetPlan_result()) {
        optionals.set(0);
      }
      if (struct.isSetExecution_time_ms()) {
        optionals.set(1);
      }
      oprot.writeBitSet(optionals, 2);
      if (struct.isSetPlan_result()) {
        oprot.writeString(struct.plan_result);
      }
      if (struct.isSetExecution_time_ms()) {
        oprot.writeI64(struct.execution_time_ms);
      }
    }

    @Override
    public void read(org.apache.thrift.protocol.TProtocol prot, TPlanResult struct) throws org.apache.thrift.TException {
      org.apache.thrift.protocol.TTupleProtocol iprot = (org.apache.thrift.protocol.TTupleProtocol) prot;
      java.util.BitSet incoming = iprot.readBitSet(2);
      if (incoming.get(0)) {
        struct.plan_result = iprot.readString();
        struct.setPlan_resultIsSet(true);
      }
      if (incoming.get(1)) {
        struct.execution_time_ms = iprot.readI64();
        struct.setExecution_time_msIsSet(true);
      }
    }
  }

  private static <S extends org.apache.thrift.scheme.IScheme> S scheme(org.apache.thrift.protocol.TProtocol proto) {
    return (org.apache.thrift.scheme.StandardScheme.class.equals(proto.getScheme()) ? STANDARD_SCHEME_FACTORY : TUPLE_SCHEME_FACTORY).getScheme();
  }
}

