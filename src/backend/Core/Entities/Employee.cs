namespace UnicornHRMS.Core.Entities
{
    public class Employee : BaseEntity
    {
        public string EmployeeCode { get; set; } = string.Empty;
        public string FirstName { get; set; } = string.Empty;
        public string LastName { get; set; } = string.Empty;
        public string Email { get; set; } = string.Empty;
        public string? PhoneNumber { get; set; }
        public DateTime DateOfBirth { get; set; }
        public DateTime HireDate { get; set; }
        public string Position { get; set; } = string.Empty;
        public decimal Salary { get; set; }
        public bool IsActive { get; set; }

        // New fields
        public int? DepartmentId { get; set; }
        public int? ManagerId { get; set; }
        public int? UserId { get; set; }
        public string? ProfilePicture { get; set; }
        public string? Address { get; set; }
        public string? City { get; set; }
        public string? Country { get; set; }
        public string? PostalCode { get; set; }

        // Navigation properties
        public Department? Department { get; set; }
        public Employee? Manager { get; set; }
        public User? User { get; set; }
        public ICollection<Employee> Subordinates { get; set; } = new List<Employee>();
        public ICollection<Attendance> Attendances { get; set; } = new List<Attendance>();
        public ICollection<LeaveRequest> LeaveRequests { get; set; } = new List<LeaveRequest>();
    }
}