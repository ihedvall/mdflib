namespace mdflibrary_test;

public static class MdfHelper
{
    public static ulong GetUnixNanoTimestamp(DateTime time)
    {

        var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
        var elapsedTime = time.ToUniversalTime().Subtract(epoch);

        var nanoSeconds =
            (ulong)elapsedTime.TotalSeconds * 1000000000UL +
            (ulong)elapsedTime.Milliseconds * 1000000UL;

        return nanoSeconds;
    }

    public static ulong GetLocalNanoTimestamp(DateTime time)
    {
        var epoch = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
        var localTime = time.ToLocalTime();
        var elapsedTime = localTime.Subtract(epoch);
        var nanoSeconds =
            (ulong)elapsedTime.TotalSeconds * 1000000000UL +
            (ulong)elapsedTime.Milliseconds * 1000000UL;

        return nanoSeconds;
    }

}