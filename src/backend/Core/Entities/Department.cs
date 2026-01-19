namespace UnicornHRMS.Core.Entities
{
    public class Department : BaseEntity
    {
        public string Name { get; set; } = string.Empty;
        public string Code { get; set; } = string.Empty;
        public string? Description { get; set; }
        public int? ManagerId { get; set; }
        public int? ParentDepartmentId { get; set; }
        public bool IsActive { get; set; } = true;

        // Navigation properties
        public Employee? Manager { get; set; }
        public Department? ParentDepartment { get; set; }
        public ICollection<Department> SubDepartments { get; set; } = new List<Department>();
        public ICollection<Employee> Employees { get; set; } = new List<Employee>();
    }
}