using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.Services.Implementation
{
    public class LocationService : ILocationService
    {
        private const double EarthRadiusKm = 6371.0;
        private const double MaxAcceptableAccuracyMeters = 100.0;

        public double CalculateDistance(double lat1, double lon1, double lat2, double lon2)
        {
            var lat1Rad = DegreesToRadians(lat1);
            var lon1Rad = DegreesToRadians(lon1);
            var lat2Rad = DegreesToRadians(lat2);
            var lon2Rad = DegreesToRadians(lon2);

            var dLat = lat2Rad - lat1Rad;
            var dLon = lon2Rad - lon1Rad;

            var a = Math.Sin(dLat / 2) * Math.Sin(dLat / 2) +
                    Math.Cos(lat1Rad) * Math.Cos(lat2Rad) *
                    Math.Sin(dLon / 2) * Math.Sin(dLon / 2);

            var c = 2 * Math.Atan2(Math.Sqrt(a), Math.Sqrt(1 - a));

            return EarthRadiusKm * c * 1000; // Distance in meters
        }

        public bool IsAccuracyAcceptable(double? accuracy)
        {
            if (!accuracy.HasValue)
                return false;

            return accuracy.Value <= MaxAcceptableAccuracyMeters;
        }

        public bool AreCoordinatesValid(double latitude, double longitude)
        {
            return latitude >= -90 && latitude <= 90 &&
                   longitude >= -180 && longitude <= 180;
        }

        public double GetMaxAcceptableAccuracy()
        {
            return MaxAcceptableAccuracyMeters;
        }

        private double DegreesToRadians(double degrees)
        {
            return degrees * Math.PI / 180.0;
        }
    }
}