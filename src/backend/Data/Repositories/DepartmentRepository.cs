using Microsoft.EntityFrameworkCore;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;

namespace UnicornHRMS.Data.Repositories
{
    public class DepartmentRepository : Repository<Department>, IDepartmentRepository
    {
        public DepartmentRepository(ApplicationDbContext context) : base(context)
        {
        }

        public async Task<Department?> GetByCodeAsync(string code)
        {
            return await _dbSet
                .Include(d => d.Manager)
                .Include(d => d.ParentDepartment)
                .FirstOrDefaultAsync(d => d.Code == code);
        }

        public async Task<IEnumerable<Department>> GetActiveDepartmentsAsync()
        {
            return await _dbSet
                .Include(d => d.Manager)
                .Include(d => d.ParentDepartment)
                .Where(d => d.IsActive)
                .OrderBy(d => d.Name)
                .ToListAsync();
        }

        public async Task<IEnumerable<Department>> GetSubDepartmentsAsync(int parentId)
        {
            return await _dbSet
                .Include(d => d.Manager)
                .Where(d => d.ParentDepartmentId == parentId)
                .OrderBy(d => d.Name)
                .ToListAsync();
        }

        public async Task<Department?> GetWithEmployeesAsync(int id)
        {
            return await _dbSet
                .Include(d => d.Manager)
                .Include(d => d.ParentDepartment)
                .Include(d => d.Employees)
                .Include(d => d.SubDepartments)
                .FirstOrDefaultAsync(d => d.Id == id);
        }
    }
}