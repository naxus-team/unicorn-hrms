namespace UnicornHRMS.Services.DTOs.Department
{
    public class DepartmentDto
    {
        public int Id { get; set; }
        public string Name { get; set; } = string.Empty;
        public string Code { get; set; } = string.Empty;
        public string? Description { get; set; }
        public int? ManagerId { get; set; }
        public string? ManagerName { get; set; }
        public int? ParentDepartmentId { get; set; }
        public string? ParentDepartmentName { get; set; }
        public bool IsActive { get; set; }
        public int EmployeeCount { get; set; }
        public DateTime CreatedAt { get; set; }
    }

    public class CreateDepartmentDto
    {
        public string Name { get; set; } = string.Empty;
        public string Code { get; set; } = string.Empty;
        public string? Description { get; set; }
        public int? ManagerId { get; set; }
        public int? ParentDepartmentId { get; set; }
    }

    public class UpdateDepartmentDto
    {
        public string Name { get; set; } = string.Empty;
        public string? Description { get; set; }
        public int? ManagerId { get; set; }
        public int? ParentDepartmentId { get; set; }
        public bool IsActive { get; set; }
    }
}