using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Core.Interfaces
{
    public interface IUserRepository : IRepository<User>
    {
        Task<User?> GetByUsernameAsync(string username);
        Task<User?> GetByEmailAsync(string email);
        Task<User?> GetByRefreshTokenAsync(string refreshToken);
        Task<bool> UsernameExistsAsync(string username);
        Task<bool> EmailExistsAsync(string email);
        Task<IEnumerable<User>> GetUsersWithRolesAsync();
        Task<User?> GetUserWithRolesAsync(int userId);
    }
}