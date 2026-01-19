using UnicornHRMS.Services.DTOs.Auth;
using UnicornHRMS.Services.DTOs.Common;

namespace UnicornHRMS.Services.Interfaces
{
    public interface IAuthService
    {
        Task<ResponseDto<LoginResponseDto>> RegisterAsync(RegisterDto dto);
        Task<ResponseDto<LoginResponseDto>> LoginAsync(LoginDto dto);
        Task<ResponseDto<LoginResponseDto>> RefreshTokenAsync(RefreshTokenDto dto);
        Task<ResponseDto<bool>> RevokeTokenAsync(int userId);
        Task<ResponseDto<bool>> ChangePasswordAsync(int userId, ChangePasswordDto dto);
        Task<ResponseDto<bool>> RequestPasswordResetAsync(string email);
        Task<ResponseDto<bool>> ResetPasswordAsync(ResetPasswordDto dto);
    }
}