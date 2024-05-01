package com.flexpushdowndb.logparser.parser;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;
import java.io.FileInputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.BufferedInputStream;
import java.io.SequenceInputStream;
import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.lang.Thread;

import com.github.shyiko.mysql.binlog.BinaryLogFileReader;
import com.github.shyiko.mysql.binlog.event.Event;
import com.github.shyiko.mysql.binlog.event.WriteRowsEventData;
import com.github.shyiko.mysql.binlog.event.deserialization.EventDeserializer;

import com.github.shyiko.mysql.binlog.io.ByteArrayOutputStream;
import com.github.shyiko.mysql.binlog.event.DeleteRowsEventData;
import com.github.shyiko.mysql.binlog.event.EventHeaderV4;
import com.github.shyiko.mysql.binlog.event.EventType;
import com.github.shyiko.mysql.binlog.event.TableMapEventData;
import com.github.shyiko.mysql.binlog.event.XidEventData;
import com.github.shyiko.mysql.binlog.event.UpdateRowsEventData;
import org.apache.avro.Schema;
import org.apache.avro.file.DataFileReader;
import org.apache.avro.file.DataFileWriter;
import org.apache.avro.generic.GenericData;
import org.apache.avro.generic.GenericDatumReader;
import org.apache.avro.generic.GenericDatumWriter;
import org.apache.avro.generic.GenericRecord;
import org.apache.avro.io.*;

public class Parser {
		public static String currentTable = null;
		public static String currentXid = null;
    	public static long last_seen = 0;
    	public static long lastReadPoint = 0;
    	public static long lastReadFileIndex = 1;
    	public static final byte[] MAGIC_HEADER = new byte[]{(byte) 0xfe, (byte) 0x62, (byte) 0x69, (byte) 0x6e};
    	private static boolean firstCall = true; //whether the deltapump is called for the first time
    	private static long totalSize = 0;
        // binlog file reader
        private static EventDeserializer eventDeserializer = new EventDeserializer();
        private static BufferedInputStream bf, bufNextFile;
        private static FileInputStream fi, nextFile;
        private static BinaryLogFileReader reader;
        private static SequenceInputStream sequenceInputStream;
        private static Schema lineorderSchema;
        private static Schema customerSchema;
        private static Schema supplierSchema;
        private static Schema partSchema;
        private static Schema dateSchema;
        private static Schema historySchema;

	/**
	 * parse the given binlog file and write all event info in the format of avro, added field in schema compared to HaTrickBench
	 * @param binlogFilePath
	 * @return serialized avro byte array; order of tables in the vector: lineorder, customer, supplier, part, date, history
	 * @throws IOException
	 */
	public static byte[][] parseBinlogFile()  {  //throws IOException
	    System.out.println("Parsing starts...!");
	    long counter = 0;
	    long number_of_writes = 0;
        long number_of_deletes = 0;
        long number_of_updates = 0;
        long customer_upd = 0;
        long deltasTimestamp = 0;

        String workingDir = System.getProperty("user.dir");

		byte[][] output = new byte[7][];

		ByteArrayOutputStream lineorderOutputStream = new ByteArrayOutputStream();
		ByteArrayOutputStream customerOutputStream = new ByteArrayOutputStream();
		ByteArrayOutputStream supplierOutputStream = new ByteArrayOutputStream();
		ByteArrayOutputStream partOutputStream = new ByteArrayOutputStream();
		ByteArrayOutputStream dateOutputStream = new ByteArrayOutputStream();
		ByteArrayOutputStream historyOutputStream = new ByteArrayOutputStream();

        String[] lineorderCol = {"lo_orderkey", "lo_linenumber",
                                 "lo_custkey", "lo_partkey",
                                 "lo_suppkey", "lo_orderdate",
                                 "lo_orderpriority","lo_shippriority",
                                 "lo_quantity", "lo_extendedprice",
                                 "lo_discount", "lo_revenue",
                                 "lo_supplycost", "lo_tax",
                                 "lo_commitdate", "lo_shipmode",
                                 "type"};

        String[] customerCol = {"c_custkey", "c_name",
                                "c_address", "c_city",
                                "c_nation", "c_region",
                                "c_phone", "c_mktsegment",
                                "c_paymentcnt", "type"};

        String[] supplierCol = {"s_suppkey", "s_name",
                                "s_address", "s_city",
                                "s_nation", "s_region",
                                "s_phone", "s_ytd",
                                "type"};

        String[] partCol = {"p_partkey", "p_name",
                            "p_mfgr", "p_category",
                            "p_brand1", "p_color",
                            "p_type", "p_size",
                            "p_container", "p_price",
                            "type"};

        String[] dateCol = {"d_datekey", "d_date",
                            "d_dayofweek", "d_month",
                            "d_year", "d_yearmonthnum",
                            "d_yearmonth",  "d_daynuminweek",
                            "d_daynuminmonth", "d_daynuminyear",
                            "d_monthnuminyear", "d_weeknuminyear",
                            "d_sellingseason", "d_lastdayinweekfl",
                            "d_lastdayinmonthfl", "d_holidayfl",
                            "d_weekdayfl", "type"};

		String[] historyCol = {"h_orderkey", "h_custkey",
		                        "h_amount", "type"};

		//First, we use a Parser to read our schema definition and create a Schema object.
		try{
            lineorderSchema = new Schema.Parser().parse(new File(workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/lineorder_d.json"));
            customerSchema = new Schema.Parser().parse(new File(workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/customer_d.json"));
            supplierSchema = new Schema.Parser().parse(new File(workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/supplier_d.json"));
            partSchema = new Schema.Parser().parse(new File(workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/part_d.json"));
            dateSchema = new Schema.Parser().parse(new File(workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/date_d.json"));
            historySchema = new Schema.Parser().parse(new File(workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/history_d.json"));
        } catch (IOException e){
            e.printStackTrace();
        }

		//avro serializer
		DatumWriter<GenericRecord> lineorderDatumWriter = new GenericDatumWriter<GenericRecord>(lineorderSchema);
		DatumWriter<GenericRecord> customerDatumWriter = new GenericDatumWriter<GenericRecord>(customerSchema);
		DatumWriter<GenericRecord> supplierDatumWriter = new GenericDatumWriter<GenericRecord>(supplierSchema);
		DatumWriter<GenericRecord> partDatumWriter = new GenericDatumWriter<GenericRecord>(partSchema);
		DatumWriter<GenericRecord> dateDatumWriter = new GenericDatumWriter<GenericRecord>(dateSchema);
		DatumWriter<GenericRecord> historyDatumWriter = new GenericDatumWriter<GenericRecord>(historySchema);


		DataFileWriter<GenericRecord> lineorderFileWriter = new DataFileWriter<GenericRecord>(lineorderDatumWriter);
		DataFileWriter<GenericRecord> customerFileWriter = new DataFileWriter<GenericRecord>(customerDatumWriter);
		DataFileWriter<GenericRecord> partFileWriter = new DataFileWriter<GenericRecord>(partDatumWriter);
		DataFileWriter<GenericRecord> dateFileWriter = new DataFileWriter<GenericRecord>(dateDatumWriter);
		DataFileWriter<GenericRecord> supplierFileWriter = new DataFileWriter<GenericRecord>(supplierDatumWriter);
		DataFileWriter<GenericRecord> historyFileWriter = new DataFileWriter<GenericRecord>(historyDatumWriter);

        try {
            lineorderFileWriter.create(lineorderSchema, lineorderOutputStream);
            customerFileWriter.create(customerSchema, customerOutputStream);
            partFileWriter.create(partSchema, partOutputStream);
            dateFileWriter.create(dateSchema, dateOutputStream);
            supplierFileWriter.create(supplierSchema, supplierOutputStream);
            historyFileWriter.create(historySchema, historyOutputStream);
        } catch (IOException e){
            e.printStackTrace();
        }
		HashMap<String, Schema> schemaList = new HashMap<String, Schema>();
		HashMap<String, String[]> colList = new HashMap<String, String[]>();
		HashMap<String, DatumWriter<GenericRecord>> dwList = new HashMap<String, DatumWriter<GenericRecord>>();
		HashMap<String, ByteArrayOutputStream> osList = new HashMap<String, ByteArrayOutputStream>();
		HashMap<String, DataFileWriter<GenericRecord>> fwList = new HashMap<String, DataFileWriter<GenericRecord>>();

		//put schemas and file writer into hash map
		//NOTE: in HATtrick benchmark all table names are in uppercase,
		//may need to modify code when encounter lowercase table names (e.g. currentTable.toLowerCase())
		schemaList.put("LINEORDER", lineorderSchema);
		schemaList.put("CUSTOMER", customerSchema);
		schemaList.put("SUPPLIER", supplierSchema);
		schemaList.put("PART", partSchema);
		schemaList.put("DATE", dateSchema);
		schemaList.put("HISTORY", historySchema);

		colList.put("LINEORDER", lineorderCol);
		colList.put("CUSTOMER", customerCol);
		colList.put("SUPPLIER", supplierCol);
		colList.put("PART", partCol);
		colList.put("DATE", dateCol);
		colList.put("HISTORY", historyCol);

		dwList.put("LINEORDER", lineorderDatumWriter);
		dwList.put("CUSTOMER", customerDatumWriter);
		dwList.put("SUPPLIER", supplierDatumWriter);
		dwList.put("PART", partDatumWriter);
		dwList.put("DATE", dateDatumWriter);
	    dwList.put("HISTORY", historyDatumWriter);

		osList.put("LINEORDER", lineorderOutputStream);
		osList.put("CUSTOMER", customerOutputStream);
		osList.put("SUPPLIER", supplierOutputStream);
		osList.put("PART", partOutputStream);
		osList.put("DATE", dateOutputStream);
		osList.put("HISTORY", historyOutputStream);

		fwList.put("LINEORDER", lineorderFileWriter);
		fwList.put("CUSTOMER", customerFileWriter);
		fwList.put("SUPPLIER", supplierFileWriter);
		fwList.put("PART", partFileWriter);
		fwList.put("DATE", dateFileWriter);
		fwList.put("HISTORY", historyFileWriter);

        long startTime = System.currentTimeMillis();
        String filePath = "/home/ubuntu/binlog/" + String.format("bin.%06d", lastReadFileIndex);
        File binlogFile = new File(filePath);
        long fileSize = binlogFile.length();
        System.out.println("file size: " + fileSize);
        try {
            System.out.println("Reading file: " + filePath + " from last read point: " + lastReadPoint);
            fi = new FileInputStream(filePath);
            fi.skip(lastReadPoint);
            bf = new BufferedInputStream(fi);

            if (firstCall) {
                reader = new BinaryLogFileReader(bf, eventDeserializer);
                firstCall = false;
                lastReadPoint = fileSize;
            } else {
                File nextLogFile = new File("/home/ubuntu/binlog/" + String.format("bin.%06d", lastReadFileIndex+1));
                if (nextLogFile.exists()) {
                    try {
                        nextFile = new FileInputStream("/home/ubuntu/binlog/" + String.format("bin.%06d", lastReadFileIndex+1));
                        bufNextFile = new BufferedInputStream(nextFile);
                    } catch (IOException e){
                        e.printStackTrace();
                    }

                    sequenceInputStream = new SequenceInputStream(bufNextFile, bf);
                    lastReadPoint = nextLogFile.length();
                    lastReadFileIndex++;
                } else {
                    sequenceInputStream = new SequenceInputStream(new ByteArrayInputStream(MAGIC_HEADER), bf);
                    lastReadPoint = fileSize;
                }

                reader = new BinaryLogFileReader(sequenceInputStream, eventDeserializer);
            }
        } catch (IOException e){
            System.out.println("[Java] Cannot instantiate reader for parsing the logs");
            e.printStackTrace();
        }
        long endTime1 = System.currentTimeMillis();
        long duration1 = (endTime1 - startTime);  // Time taken in nanoseconds
        double seconds1 = (double) duration1 / 1000; // Convert to seconds

        System.out.println("Execution time of initials: " + seconds1 + " seconds");

		eventDeserializer.setCompatibilityMode(
		    EventDeserializer.CompatibilityMode.DATE_AND_TIME_AS_LONG,
			EventDeserializer.CompatibilityMode.CHAR_AND_BINARY_AS_BYTE_ARRAY
		);

		try {
			for (Event event; (event = reader.readEvent()) != null;) {
				EventHeaderV4 header = event.getHeader();
				totalSize +=  header.getHeaderLength() + header.getDataLength();
				if(totalSize>fileSize) {
				    break;
				}

				EventType type = header.getEventType();
				deltasTimestamp = header.getTimestamp();
				/*if(type == EventType.XID){
				    long xid =  ((XidEventData) event.getData()).getXid();
				    System.out.println("[Java] Event type XID: "+xid);
				}*/

				if (type == EventType.TABLE_MAP) {
					currentTable = ((TableMapEventData) event.getData()).getTable();
					continue;
				}

                if(currentTable == null)
                    continue;

                // Get for currentTable record, dataFileWriter, colList and datumWriter.
				GenericRecord record = new GenericData.Record(schemaList.get(currentTable));
				DataFileWriter<GenericRecord> dataFileWriter = fwList.get(currentTable);
				String[] colName = colList.get(currentTable);
				DatumWriter<GenericRecord> datumWriter = dwList.get(currentTable);

				if (EventType.isWrite(type)) {
					WriteRowsEventData data = (WriteRowsEventData) event.getData();
					List<Serializable[]> list = data.getRows();
					//for every row in the list, put each row into one string array
					for (Serializable[] row : list) {
						int writeIndex = 0;
						for (Serializable val : row) {
							String value;
							if (val instanceof byte[]) {
								value = new String((byte[]) val, StandardCharsets.UTF_8);
								record.put(colName[writeIndex++], value);
							} else {
								record.put(colName[writeIndex++], val);
							}
						}
						record.put(colName[writeIndex++], "INSERT");
					}
					number_of_writes++;
				} else if (EventType.isDelete(type)) {
					DeleteRowsEventData data = (DeleteRowsEventData) event.getData();
					List<Serializable[]> list = data.getRows();
					// TODO: handle deletes
					for (Serializable[] row : list) {
						int deleteIndex = 0;
						for (Serializable val : row) {
							String value;
							if (val instanceof byte[]) {
								value = new String((byte[]) val, StandardCharsets.UTF_8);
								record.put(colName[deleteIndex++], value);
							} else {
								record.put(colName[deleteIndex++], val);
							}
						}
						record.put(colName[deleteIndex++], "DELETE");
					}
					number_of_deletes++;
				} else
				 if (EventType.isUpdate(type)) {
					UpdateRowsEventData data = (UpdateRowsEventData) event.getData();
					List<Map.Entry<Serializable[], Serializable[]>> list = data.getRows();
					for (Map.Entry<Serializable[], Serializable[]> entry : list) {
						Serializable[] row = entry.getValue();
						int updateIndex = 0;
						for (Serializable val : row) {
							String value;
							if (val instanceof byte[]) {
								value = new String((byte[]) val, StandardCharsets.UTF_8);
								record.put(colName[updateIndex++], value);
							} else {
								record.put(colName[updateIndex++], val);
							}
						}
						record.put(colName[updateIndex++], "UPDATE");
					}
					number_of_updates++;
				}  else {
				    continue;
				}
				dataFileWriter.append(record);
				dataFileWriter.flush();
				counter++;
			}
			double endTime = System.currentTimeMillis();
			double duration = (endTime - startTime) / 1000;
		} catch (OutOfMemoryError e) {
              //Log the info
              System.err.println("Out of Memory");
              System.err.println("Max JVM memory: " + Runtime.getRuntime().maxMemory());
        } catch (Exception e) {
			e.printStackTrace();
		} finally {
			for(ByteArrayOutputStream o : osList.values()) {
				if(o != null) {
                    try{
                        o.close();
                    } catch (IOException e){
                        e.printStackTrace();
                    }
				}
			}
			for(DataFileWriter d : fwList.values()){
			    if(d != null){
			        try {
			            d.close();
			        } catch (IOException e){
                       e.printStackTrace();
                    }
			    }
			}
		}

        long endTime = System.currentTimeMillis();

        long duration = (endTime - startTime);  // Time taken in nanoseconds
        double seconds = (double) duration / 1000; // Convert to seconds

        System.out.println("Execution time of loop of events: " + seconds + " seconds");
        System.out.println("Total size of events (file) so far: " + totalSize);

        System.out.println("Parsing OK! "+"INS: "+number_of_writes+", UPD:"+number_of_updates+", DEL:"+number_of_deletes+", "+deltasTimestamp);
		//if no event for certain table, the length of its output stream would be 0
		output[0] = lineorderOutputStream.toByteArray();
		output[1] = customerOutputStream.toByteArray();
		output[2] = supplierOutputStream.toByteArray();
		output[3] = partOutputStream.toByteArray();
		output[4] = dateOutputStream.toByteArray();
		output[5] = historyOutputStream.toByteArray();
		output[6] = ByteBuffer.allocate(8).putLong(deltasTimestamp).array();  // last entry for the timestamp
		return output;
	}
}