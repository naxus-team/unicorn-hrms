namespace UnicornHRMS.Core.Entities
{
    public class Attendance : BaseEntity
    {
        public int EmployeeId { get; set; }
        public DateTime Date { get; set; }
        public TimeSpan? CheckIn { get; set; }
        public TimeSpan? CheckOut { get; set; }
        public AttendanceStatus Status { get; set; }
        public string? Notes { get; set; }

        // GPS Coordinates for Check-In
        public double? CheckInLatitude { get; set; }
        public double? CheckInLongitude { get; set; }
        public double? CheckInAccuracy { get; set; } // In meters

        // GPS Coordinates for Check-Out
        public double? CheckOutLatitude { get; set; }
        public double? CheckOutLongitude { get; set; }
        public double? CheckOutAccuracy { get; set; } // In meters

        // Location validation flags
        public bool IsCheckInLocationValid { get; set; } = true;
        public bool IsCheckOutLocationValid { get; set; } = true;
        public string? LocationNotes { get; set; } // For tracking location issues

        // Navigation property
        public Employee Employee { get; set; } = null!;
    }

    public enum AttendanceStatus
    {
        Present,
        Absent,
        Late,
        HalfDay,
        OnLeave
    }
}