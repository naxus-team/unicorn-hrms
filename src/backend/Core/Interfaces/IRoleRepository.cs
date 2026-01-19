using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Core.Interfaces
{
    public interface IRoleRepository : IRepository<Role>
    {
        Task<Role?> GetByNameAsync(string name);
        Task<IEnumerable<Role>> GetRolesByUserIdAsync(int userId);
    }
}