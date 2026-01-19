namespace UnicornHRMS.Services.DTOs.Employee
{
    public class EmployeeDto
    {
        public int Id { get; set; }
        public string EmployeeCode { get; set; } = string.Empty;
        public string FirstName { get; set; } = string.Empty;
        public string LastName { get; set; } = string.Empty;
        public string FullName => $"{FirstName} {LastName}";
        public string Email { get; set; } = string.Empty;
        public string? PhoneNumber { get; set; }
        public DateTime DateOfBirth { get; set; }
        public DateTime HireDate { get; set; }
        public string Position { get; set; } = string.Empty;
        public decimal Salary { get; set; }
        public bool IsActive { get; set; }

        // New fields
        public int? DepartmentId { get; set; }
        public string? DepartmentName { get; set; }
        public int? ManagerId { get; set; }
        public string? ManagerName { get; set; }
        public int? UserId { get; set; }
        public string? Address { get; set; }
        public string? City { get; set; }
        public string? Country { get; set; }
        public string? PostalCode { get; set; }
        public string? ProfilePicture { get; set; }

        public DateTime CreatedAt { get; set; }
        public DateTime? UpdatedAt { get; set; }
    }

    public class CreateEmployeeDto
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

        // New fields
        public int? DepartmentId { get; set; }
        public int? ManagerId { get; set; }
        public string? Address { get; set; }
        public string? City { get; set; }
        public string? Country { get; set; }
        public string? PostalCode { get; set; }
    }

    public class UpdateEmployeeDto
    {
        public string FirstName { get; set; } = string.Empty;
        public string LastName { get; set; } = string.Empty;
        public string Email { get; set; } = string.Empty;
        public string? PhoneNumber { get; set; }
        public string Position { get; set; } = string.Empty;
        public decimal Salary { get; set; }
        public bool IsActive { get; set; }

        // New fields
        public int? DepartmentId { get; set; }
        public int? ManagerId { get; set; }
        public string? Address { get; set; }
        public string? City { get; set; }
        public string? Country { get; set; }
        public string? PostalCode { get; set; }
    }
}