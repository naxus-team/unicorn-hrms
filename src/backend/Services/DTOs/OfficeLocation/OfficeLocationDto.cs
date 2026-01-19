namespace UnicornHRMS.Services.DTOs.OfficeLocation
{
    public class GeoFenceValidationResult
    {
        public bool IsWithinGeofence { get; set; }
        public double DistanceFromOffice { get; set; }
        public string OfficeLocationName { get; set; } = string.Empty;
        public string Message { get; set; } = string.Empty;
    }
}