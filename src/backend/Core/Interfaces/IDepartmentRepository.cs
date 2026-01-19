using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Core.Interfaces
{
    public interface IDepartmentRepository : IRepository<Department>
    {
        Task<Department?> GetByCodeAsync(string code);
        Task<IEnumerable<Department>> GetActiveDepartmentsAsync();
        Task<IEnumerable<Department>> GetSubDepartmentsAsync(int parentId);
        Task<Department?> GetWithEmployeesAsync(int id);
    }
}