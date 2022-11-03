
namespace mdflibrary_test;

using MdfLibrary;
using System;

[TestClass]
public class MdfLibraryTest
{
    private const string TestFile1 = "K:/test/mdf/mdf4_1/Simple/ETAS_SimpleSorted.mf4";
    private const string TestFile2 = "K:/test/mdf/50B8FE41-00000040-00000002-6329B255.MF4";
    private const string InvalidFile = "K:/test/dbc/Backbone1.CEA2p.dbc";
    
    [TestMethod]
    public void TestStatic()
    {
        var instance = MdfLibrary.Instance;
        Assert.IsNotNull(instance);
            
        var mdf = MdfLibrary.IsMdfFile(TestFile1);
        Assert.IsTrue(mdf);
        
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());

        var datagroup = reader.get_DataGroup(0); 
        Assert.IsNotNull(datagroup);

        var observerList = new List<MdfChannelObserver>(); 
        var groupList = datagroup.ChannelGroups;
        foreach (var group in groupList)
        {
            Console.WriteLine("Group: {0}, Desc: {1}, Samples: {2:D}", 
                 group.Name,
                 group.Description,
                 group.NofSamples);
            var tempList = MdfLibrary.CreateChannelObserverForChannelGroup(
             datagroup, group);
            Assert.IsNotNull(tempList);
            Assert.AreNotEqual(tempList.Length, 0);
            foreach (var observer in tempList)
            { 
                Console.WriteLine("Observer: {0}, Unit: {1}", 
                    observer.Name,
                    observer.Unit); 
                observerList.Add(observer);
            }
        }
        Assert.IsTrue(reader.ReadData(datagroup));
        foreach (var channel in observerList)
        {
            Console.WriteLine("Channel: {0}, Master: {1}, Samples: {2} ",
                channel.Name, channel.IsMaster(), channel.NofSamples);
            var value1 = "";
            for (UInt64 sample = 20;
                 sample < 30 && sample < channel.NofSamples;
                 ++sample)
            {
                var valid = channel.GetChannelValueAsString(sample, ref value1);
                Console.WriteLine("Sample: {0}:{1} {2})",
                    sample, value1, valid);
            }
        }
    }

    [TestMethod]
    public void TestNormalRead()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsNotNull(reader);

        var readHeader = reader.ReadHeader();
        Assert.IsTrue(readHeader);

        var readInfo = reader.ReadMeasurementInfo();
        Assert.IsTrue(readInfo);

        var readAll = reader.ReadEverythingButData();
        Assert.IsTrue(readAll);
    }

    [TestMethod]
    public void TestReader()
    {
        var reader1 = new MdfReader(TestFile1);
        Assert.IsNotNull(reader1);
        reader1.Index = 666;
        Assert.AreEqual(reader1.Index, 666);
        
        Console.WriteLine("Name: {0}", reader1.Name);
        Assert.IsNotNull(reader1.File);

        // As we not read yet, the following functions should return null
        Assert.IsNull(reader1.Header);
        Assert.IsNull(reader1.get_DataGroup(0));
        
        Assert.IsTrue(reader1.IsOk);
        reader1.Close();
        Assert.IsTrue(reader1.Open());
        
        Assert.IsTrue(reader1.ReadHeader());
        Assert.IsTrue(reader1.ReadMeasurementInfo()); 
        Assert.IsTrue(reader1.ReadEverythingButData());        
        
        Assert.IsNotNull(reader1.Header);
        Assert.IsNotNull(reader1.get_DataGroup(0));        
        Assert.IsTrue(reader1.IsOk);
        reader1.Close();

        var reader2 = new MdfReader(InvalidFile);
        Assert.IsFalse(reader2.IsOk);
        Assert.IsNull(reader2.File);
     }

    [TestMethod]
    public void TestFile()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData()) ;
        reader.Close();
        
        var file = reader.File;
        Assert.IsNotNull(file);

        var attachments = file.Attachments;
        Assert.IsNotNull(attachments);
        Assert.AreEqual(attachments.Length, 0);
        
        var datagroups = file.DataGroups;
        Assert.IsNotNull(datagroups);
        Assert.AreEqual(datagroups.Length, 2);
        
        Console.WriteLine("Name: {0}", file.Name);
        Console.WriteLine("Filename: {0}", file.Filename); 
        Console.WriteLine("Version: {0}", file.Version);            
        Console.WriteLine("Main Version: {0}", file.MainVersion); 
        Console.WriteLine("Minor Version: {0}", file.MinorVersion);
        Console.WriteLine("Program Id: {0}", file.ProgramId);
        
        Assert.IsNotNull(file.Header);
        Assert.IsTrue(file.Finalized);        
        Assert.IsTrue(file.IsMdf4);
        
        Assert.IsNotNull(file.CreateAttachment());
        attachments = file.Attachments;
        Assert.AreEqual(attachments.Length, 1);
        
        Assert.IsNotNull(file.CreateDataGroup());        
        datagroups = file.DataGroups;
        Assert.AreEqual(datagroups.Length, 3);
        
        Assert.IsNull(file.FindParentDataGroup(null));
    }

    [TestMethod]
    public void TestHeader()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData()) ;
        reader.Close();

        var header = reader.Header;
        Assert.IsNotNull(header);
        
        Assert.AreEqual(header.Index, 64);
        Console.WriteLine("Description: {0}", header.Description);
        Console.WriteLine("Author: {0}", header.Author);        
        Console.WriteLine("Department: {0}", header.Department); 
        Console.WriteLine("Project: {0}", header.Project);
        Console.WriteLine("Subject: {0}", header.Subject);
        Console.WriteLine("Measurement ID: {0}", header.MeasurementId);
        Console.WriteLine("Recorder ID: {0}", header.RecorderId);
        Console.WriteLine("Recorder Index: {0}", header.RecorderIndex); 
        Console.WriteLine("Start Time {0}", header.StartTime);
        Console.WriteLine("Start Angle {0}", header.StartAngle);        
        Console.WriteLine("Start Distance {0}", header.StartDistance);  
        
        Assert.IsNotNull(header.MetaData);

        var attachments = header.Attachments;
        Assert.IsNotNull(attachments);
        Assert.AreEqual(attachments.Length, 0);

        var histories = header.FileHistories;
        Assert.IsNotNull(histories);
        Assert.AreEqual(histories.Length, 1);

        var events = header.Events;
        Assert.IsNotNull(events);
        Assert.AreEqual(events.Length, 0); 
        
        var groups = header.DataGroups;
        Assert.IsNotNull(groups);
        Assert.AreEqual(groups.Length, 2);  
        
        Assert.IsFalse(header.IsStartAngleUsed);
        Assert.IsFalse(header.IsStartDistanceUsed);
        Assert.IsNotNull(header.LastDataGroup);
        
        Assert.IsNotNull(header.CreateAttachment());
        Assert.IsNotNull(header.CreateFileHistory()); 
        Assert.IsNotNull(header.CreateMdfEvent());        
        Assert.IsNotNull(header.CreateDataGroup());  
    }

    [TestMethod]
    public void TestHistory()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData()) ;
        reader.Close();

        var header = reader.Header;
        var list = header.FileHistories;
        var history = list[0];
        Assert.IsNotNull(history);
        
        Console.WriteLine("Index: {0}", history.Index);
        Console.WriteLine("Time: {0}", history.Time);
        Console.WriteLine("Description: {0}", history.Description);
        Console.WriteLine("Tool Name: {0}", history.ToolName);
        Console.WriteLine("Tool Vendor: {0}", history.ToolVendor);        
        Console.WriteLine("Tool Version: {0}", history.ToolVersion);
        Console.WriteLine("User Name: {0}", history.UserName);
        
        Assert.IsNotNull(history.MetaData);
    }

    [TestMethod]
    public void TestMetaData()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData()) ;
        reader.Close();
 
         var header = reader.Header;
         var meta = header.MetaData;
         Assert.IsNotNull(meta);     
         
         Console.WriteLine("TX: {0}", meta.get_PropertyAsString("TX"));
         Console.WriteLine("Time Source: {0}", meta.get_PropertyAsString("time_source"));
         
         var props = meta.Properties;
         Assert.IsNotNull(props);
         foreach (var prop in props) 
         {
             Console.WriteLine( 
                 "Name: {0}, Value: {1}" , 
                 prop.Name, 
                 prop.ValueAsString);
         }
         
         var commons = meta.CommonProperties;
         Assert.IsNotNull(commons);
         foreach (var tag in commons) 
         {
             Console.WriteLine( 
                 "Name: {0}, Desc: {1}, Unit: {2}, Ref: {3}, Type: {4}, Lang: {5}, R/O: {6}, Value: {7}" , 
                 tag.Name, 
                 tag.Description,
                 tag.Unit,
                 tag.UnitRef,
                 tag.Type,
                 tag.Language,
                 tag.ReadOnly,
                 tag.ValueAsString);
         }
    }

    [TestMethod]
    public void TestDataGroup()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData()) ;
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var group = header.DataGroups[0];
        Assert.IsNotNull(group);
        
        Console.WriteLine("Index: {0}", group.Index);   
        Console.WriteLine("Desc: {0}", group.Description);
        Console.WriteLine("Record ID size: {0}", group.RecordIdSize);
        Assert.IsNotNull(group.ChannelGroups);
        Assert.IsNull(group.MetaData);
        Assert.IsFalse(group.IsRead);
        
        Assert.IsNotNull(group.CreateChannelGroup());
        Assert.IsNotNull(group.CreateMetaData());
        Assert.IsNull(group.FindParentChannelGroup(null));
        group.ResetSample();
    }

    [TestMethod]
    public void TestChannelGroup()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        Assert.IsNotNull(group);

        Console.WriteLine("Index: {0}", group.Index);
        Console.WriteLine("Record ID: {0}", group.RecordId);
        Console.WriteLine("Name: {0}", group.Name);
        Console.WriteLine("Desc: {0}", group.Description);
        Console.WriteLine("Saamples: {0}", group.NofSamples);
        Console.WriteLine("Flags: {0:X}", group.Flags);
        Console.WriteLine("Sep: {0}", group.PathSeparator.ToString());

        Assert.IsNotNull(group.Channels);
        Assert.IsNull(group.SourceInformation);
        Assert.IsNull(group.GetXChannel(null));
    }

    [TestMethod]
    public void TestChannel()
    {
        var reader = new MdfReader(TestFile1);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        var channel = group.Channels[0];
        Assert.IsNotNull(channel); 
        
        Console.WriteLine("Index: {0}", channel.Index);
        Console.WriteLine("Name: {0}", channel.Name); 
        Console.WriteLine("Display Name: {0}", channel.DisplayName);                
        Console.WriteLine("Description: {0}", channel.Description);          
        Console.WriteLine("Unit Used: {0}", channel.UnitUsed); 
        Console.WriteLine("Unit: {0}", channel.Unit);         
        Console.WriteLine("Channel Type: {0}", channel.Type);          
        Console.WriteLine("Sync Type: {0}", channel.Sync);              
        Console.WriteLine("Data Type: {0}", channel.DataType);
        Console.WriteLine("Data Bytes: {0}", channel.DataBytes);        
        Console.WriteLine("Precision Used: {0}", channel.PrecisionUsed); 
        Console.WriteLine("Precision: {0}", channel.Precision); 
        
        Console.WriteLine("Range Used: {0}", channel.RangeUsed); 
        Console.WriteLine("Range Min: {0}", channel.RangeMin);
        Console.WriteLine("Range Max: {0}", channel.RangeMax);
        
        Console.WriteLine("Limit Used: {0}", channel.LimitUsed); 
        Console.WriteLine("Limit Min: {0}", channel.LimitMin);
        Console.WriteLine("Limit Max: {0}", channel.LimitMax);  
        
        Console.WriteLine("Ext Limit Used: {0}", channel.ExtLimitUsed); 
        Console.WriteLine("Ext Limit Min: {0}", channel.ExtLimitMin);
        Console.WriteLine("Ext Limit Max: {0}", channel.ExtLimitMax);    
        
        Console.WriteLine("Sampling Rate: {0}", channel.SamplingRate);
        
        Assert.IsNull(channel.ChannelConversion);
    }

    [TestMethod]
    public void TestSourceInformation()
    {
        var reader = new MdfReader(TestFile2);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        var info = group.SourceInformation;
        Assert.IsNotNull(info);
        
        Console.WriteLine("Index: {0}", info.Index);
        Console.WriteLine("Name: {0}", info.Name);
        Console.WriteLine("Desc: {0}", info.Description);
        Console.WriteLine("Path: {0}", info.Path);                
        Console.WriteLine("Type: {0}", info.Type);            
        Console.WriteLine("Bus: {0}", info.Bus);
        Console.WriteLine("Flags: {0:X}", info.Flags);
        
        Assert.IsNotNull(info.MetaData);
    }

    [TestMethod]
    public void TestChannelConversion()
    {
        var reader = new MdfReader(TestFile2);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        var channel = group.Channels[0];
        var conv = channel.ChannelConversion;
        Assert.IsNotNull(conv);
        
        Console.WriteLine("Index: {0:X}", conv.Index);
        Console.WriteLine("Name: {0}", conv.Name);
        Console.WriteLine("Desc: {0}", conv.Description);
        Console.WriteLine("Unit Used: {0}", conv.UnitUsed);
        Console.WriteLine("Unit: {0}", conv.Unit);                
        Console.WriteLine("Type: {0}", conv.Type);         
        Console.WriteLine("Precision Used: {0}", conv.PrecisionUsed);
        Console.WriteLine("Precision: {0}", conv.Precision);
        Console.WriteLine("Range Used: {0}", conv.RangeUsed);
        Console.WriteLine("Range Min: {0}", conv.RangeMin);
        Console.WriteLine("Range Max: {0}", conv.RangeMax);
        Console.WriteLine("Flags: {0:X}", conv.Flags);        
        Assert.IsNull(conv.Inverse);
        Assert.IsNotNull(conv.CreateInverse());
    }

    [TestMethod]
    public void TestCanMessage()
    {
        var reader = new MdfReader(TestFile2);
        Assert.IsTrue(reader.ReadEverythingButData());
        reader.Close();

        var file = reader.File;
        var header = file.Header;
        var datagroup = header.DataGroups[0];
        var group = datagroup.ChannelGroups[0];
        List<MdfChannelObserver> list = new List<MdfChannelObserver>();
        
        var time = MdfLibrary.CreateChannelObserverByChannelName(datagroup, 
            "Timestamp");
        Assert.IsNotNull(time);
        var ident = MdfLibrary.CreateChannelObserverByChannelName(datagroup, 
            "CAN_DataFrame.ID");
        Assert.IsNotNull(ident);        
        var data = MdfLibrary.CreateChannelObserverByChannelName(datagroup, 
            "CAN_DataFrame.DataBytes");
        Assert.IsNotNull(data);

        list.Add(time);
        list.Add(ident);
        list.Add(data);

        reader.Open();
        reader.ReadData(datagroup);
        reader.Close();

        double time_value = 0;
        bool time_valid = false;
        
        ulong ident_value = 0;
        bool ident_valid = false;
        
        byte[] data_value = new byte[8];
        bool data_valid = false;
        
        string data_string = "";
                
        for (ulong sample = 0; sample < 10; ++sample)
        {
            time_valid = time.GetEngValueAsFloat(sample, ref time_value);
            ident_valid = ident.GetEngValueAsUnsigned(sample, ref ident_value);            
            data_valid = data.GetEngValueAsArray(sample, ref data_value);
            data.GetEngValueAsString(sample, ref data_string);              
            Console.WriteLine("Valid: {0} {1} {2} {3}", sample, time_valid, ident_valid,data_valid);          
            Console.WriteLine("Sample: {0} {1} {2} {3}", sample, time_value, ident_value, data_string);
            Console.Write("Sample: {0} ", sample);
            foreach (var val in data_value)
            {
                Console.Write("{0:X2} ", val);
            }
            Console.WriteLine("");
        }



    }
}