using Microsoft.EntityFrameworkCore;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;

namespace UnicornHRMS.Data.Repositories
{
    public class EmployeeRepository : Repository<Employee>, IEmployeeRepository
    {
        public EmployeeRepository(ApplicationDbContext context) : base(context)
        {
        }

        public async Task<Employee?> GetByEmailAsync(string email)
        {
            return await _dbSet
                .Include(e => e.Department)
                .Include(e => e.Manager)
                .Include(e => e.User)
                .FirstOrDefaultAsync(e => e.Email.ToLower() == email.ToLower());
        }

        public async Task<Employee?> GetByEmployeeCodeAsync(string employeeCode)
        {
            return await _dbSet
                .Include(e => e.Department)
                .Include(e => e.Manager)
                .Include(e => e.User)
                .FirstOrDefaultAsync(e => e.EmployeeCode == employeeCode);
        }

        public async Task<IEnumerable<Employee>> GetActiveEmployeesAsync()
        {
            return await _dbSet
                .Include(e => e.Department)
                .Include(e => e.Manager)
                .Where(e => e.IsActive)
                .OrderBy(e => e.FirstName)
                .ToListAsync();
        }

        public async Task<IEnumerable<Employee>> GetByDepartmentAsync(string departmentName)
        {
            return await _dbSet
                .Include(e => e.Department)
                .Include(e => e.Manager)
                .Include(e => e.User)
                .Where(e => e.Department != null && e.Department.Name == departmentName && e.IsActive)
                .OrderBy(e => e.FirstName)
                .ToListAsync();
        }

        public override async Task<Employee?> GetByIdAsync(int id)
        {
            return await _dbSet
                .Include(e => e.Department)
                .Include(e => e.Manager)
                .Include(e => e.User)
                .Include(e => e.Attendances)
                .Include(e => e.LeaveRequests)
                .FirstOrDefaultAsync(e => e.Id == id);
        }

        public override async Task<IEnumerable<Employee>> GetAllAsync()
        {
            return await _dbSet
                .Include(e => e.Department)
                .Include(e => e.Manager)
                .Include(e => e.User)
                .OrderBy(e => e.FirstName)
                .ToListAsync();
        }
    }
}