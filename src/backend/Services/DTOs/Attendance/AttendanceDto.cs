using UnicornHRMS.Core.Entities;
using System.ComponentModel.DataAnnotations;

namespace UnicornHRMS.Services.DTOs.Attendance
{
    public class AttendanceDto
    {
        public int Id { get; set; }
        public int EmployeeId { get; set; }
        public string EmployeeName { get; set; } = string.Empty;
        public DateTime Date { get; set; }
        public TimeSpan? CheckIn { get; set; }
        public TimeSpan? CheckOut { get; set; }
        public string Status { get; set; } = string.Empty;
        public string? Notes { get; set; }

        // GPS Data
        public double? CheckInLatitude { get; set; }
        public double? CheckInLongitude { get; set; }
        public double? CheckInAccuracy { get; set; }
        public double? CheckOutLatitude { get; set; }
        public double? CheckOutLongitude { get; set; }
        public double? CheckOutAccuracy { get; set; }

        // Location Validation
        public bool IsCheckInLocationValid { get; set; }
        public bool IsCheckOutLocationValid { get; set; }
        public string? LocationNotes { get; set; }

        public TimeSpan? WorkDuration
        {
            get
            {
                if (CheckIn.HasValue && CheckOut.HasValue)
                {
                    return CheckOut.Value - CheckIn.Value;
                }
                return null;
            }
        }
    }

    public class CreateAttendanceDto
    {
        public int EmployeeId { get; set; }
        public DateTime Date { get; set; }
        public TimeSpan? CheckIn { get; set; }
        public TimeSpan? CheckOut { get; set; }
        public AttendanceStatus Status { get; set; }
        public string? Notes { get; set; }

        // GPS Data (Optional for manual attendance creation)
        public double? CheckInLatitude { get; set; }
        public double? CheckInLongitude { get; set; }
        public double? CheckInAccuracy { get; set; }
        public double? CheckOutLatitude { get; set; }
        public double? CheckOutLongitude { get; set; }
        public double? CheckOutAccuracy { get; set; }
    }

    public class UpdateAttendanceDto
    {
        public TimeSpan? CheckIn { get; set; }
        public TimeSpan? CheckOut { get; set; }
        public AttendanceStatus Status { get; set; }
        public string? Notes { get; set; }

        // GPS Data (Optional)
        public double? CheckInLatitude { get; set; }
        public double? CheckInLongitude { get; set; }
        public double? CheckOutLatitude { get; set; }
        public double? CheckOutLongitude { get; set; }
    }

    public class CheckInDto
    {
        [Required]
        public int EmployeeId { get; set; }

        [Required]
        public DateTime Date { get; set; }

        [Required]
        public TimeSpan CheckIn { get; set; }

        // GPS Coordinates (Required for location tracking)
        [Required(ErrorMessage = "Latitude is required for GPS tracking")]
        [Range(-90, 90, ErrorMessage = "Latitude must be between -90 and 90")]
        public double Latitude { get; set; }

        [Required(ErrorMessage = "Longitude is required for GPS tracking")]
        [Range(-180, 180, ErrorMessage = "Longitude must be between -180 and 180")]
        public double Longitude { get; set; }

        [Range(0, 1000, ErrorMessage = "Accuracy must be between 0 and 1000 meters")]
        public double? Accuracy { get; set; } // In meters

        public string? Notes { get; set; }
    }

    public class CheckOutDto
    {
        [Required]
        public int EmployeeId { get; set; }

        [Required]
        public DateTime Date { get; set; }

        [Required]
        public TimeSpan CheckOut { get; set; }

        // GPS Coordinates (Required for location tracking)
        [Required(ErrorMessage = "Latitude is required for GPS tracking")]
        [Range(-90, 90, ErrorMessage = "Latitude must be between -90 and 90")]
        public double Latitude { get; set; }

        [Required(ErrorMessage = "Longitude is required for GPS tracking")]
        [Range(-180, 180, ErrorMessage = "Longitude must be between -180 and 180")]
        public double Longitude { get; set; }

        [Range(0, 1000, ErrorMessage = "Accuracy must be between 0 and 1000 meters")]
        public double? Accuracy { get; set; } // In meters

        public string? Notes { get; set; }
    }
}