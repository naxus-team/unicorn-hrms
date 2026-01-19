namespace UnicornHRMS.Services.Interfaces
{
    public interface ILocationService
    {
        double CalculateDistance(double lat1, double lon1, double lat2, double lon2);
        bool IsAccuracyAcceptable(double? accuracy);
        bool AreCoordinatesValid(double latitude, double longitude);
        double GetMaxAcceptableAccuracy();
    }
}