using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.User;

namespace UnicornHRMS.Services.Interfaces
{
    public interface IUserService
    {
        Task<ResponseDto<UserDto>> GetByIdAsync(int id);
        Task<ResponseDto<List<UserDto>>> GetAllAsync();
        Task<ResponseDto<UserDto>> CreateAsync(CreateUserDto dto);
        Task<ResponseDto<UserDto>> UpdateAsync(int id, UpdateUserDto dto);
        Task<ResponseDto<bool>> DeleteAsync(int id);
        Task<ResponseDto<bool>> AssignRoleAsync(AssignRoleDto dto);
        Task<ResponseDto<bool>> RemoveRoleAsync(AssignRoleDto dto);
        Task<ResponseDto<List<string>>> GetUserRolesAsync(int userId);
    }
}