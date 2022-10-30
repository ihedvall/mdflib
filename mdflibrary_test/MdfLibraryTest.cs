namespace mdflibrary_test;

using MdfLibrary;

[TestClass]
public class MdfLibraryTest
{
    private const string TestFilename = "K:/test/mdf/mdf4_1/Simple/ETAS_SimpleSorted.mf4";

    [TestMethod]
    public void TestStatic()
    {        
        var mdf = MdfLibrary.IsMdfFile(TestFilename);
        Assert.IsTrue(mdf);
    }

    [TestMethod]
    public void TestNormalRead()
    {
        var reader = new MdfReader(TestFilename);
        Assert.IsNotNull(reader);

        var readHeader = reader.ReadHeader();
        Assert.IsTrue(readHeader);

        var readInfo = reader.ReadMeasurementInfo();
        Assert.IsTrue(readInfo);

        var readAll = reader.ReadEverythingButData();
        Assert.IsTrue(readAll);


    }
}