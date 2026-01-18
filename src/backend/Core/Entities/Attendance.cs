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