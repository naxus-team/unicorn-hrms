using AutoMapper;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;
using UnicornHRMS.Services.DTOs.Auth;
using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.User;
using UnicornHRMS.Services.Helpers;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.Services.Implementation
{
    public class AuthService : IAuthService
    {
        private readonly IUnitOfWork _unitOfWork;
        private readonly IMapper _mapper;
        private readonly ITokenService _tokenService;

        public AuthService(IUnitOfWork unitOfWork, IMapper mapper, ITokenService tokenService)
        {
            _unitOfWork = unitOfWork;
            _mapper = mapper;
            _tokenService = tokenService;
        }

        public async Task<ResponseDto<LoginResponseDto>> RegisterAsync(RegisterDto dto)
        {
            try
            {
                // Validate passwords match
                if (dto.Password != dto.ConfirmPassword)
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Passwords do not match");
                }

                // Check if username exists
                if (await _unitOfWork.Users.UsernameExistsAsync(dto.Username))
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Username already exists");
                }

                // Check if email exists
                if (await _unitOfWork.Users.EmailExistsAsync(dto.Email))
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Email already exists");
                }

                // Create user
                var user = new User
                {
                    Username = dto.Username,
                    Email = dto.Email,
                    FirstName = dto.FirstName,
                    LastName = dto.LastName,
                    PhoneNumber = dto.PhoneNumber,
                    PasswordHash = PasswordHelper.HashPassword(dto.Password),
                    IsActive = true,
                    EmailConfirmed = false
                };

                await _unitOfWork.Users.AddAsync(user);
                await _unitOfWork.SaveChangesAsync();

                // Generate tokens (role assignment will be done manually by admin later)
                var roles = new List<string> { RoleNames.Employee };
                var accessToken = _tokenService.GenerateAccessToken(user, roles);
                var refreshToken = _tokenService.GenerateRefreshToken();

                // Save refresh token
                user.RefreshToken = refreshToken;
                user.RefreshTokenExpiryTime = DateTime.UtcNow.AddDays(7);
                user.LastLoginAt = DateTime.UtcNow;
                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                var userDto = new UserDto
                {
                    Id = user.Id,
                    Username = user.Username,
                    Email = user.Email,
                    FirstName = user.FirstName,
                    LastName = user.LastName,
                    PhoneNumber = user.PhoneNumber,
                    IsActive = user.IsActive,
                    EmailConfirmed = user.EmailConfirmed,
                    Roles = roles
                };

                var response = new LoginResponseDto
                {
                    AccessToken = accessToken,
                    RefreshToken = refreshToken,
                    ExpiresAt = DateTime.UtcNow.AddMinutes(60),
                    User = userDto
                };

                return ResponseDto<LoginResponseDto>.SuccessResponse(response, "Registration successful. Please contact admin to assign roles.");
            }
            catch (Exception ex)
            {
                return ResponseDto<LoginResponseDto>.FailureResponse($"Error during registration: {ex.Message}");
            }
        }

        public async Task<ResponseDto<LoginResponseDto>> LoginAsync(LoginDto dto)
        {
            try
            {
                // Find user by username or email
                var user = await _unitOfWork.Users.GetByUsernameAsync(dto.UsernameOrEmail);
                if (user == null)
                {
                    user = await _unitOfWork.Users.GetByEmailAsync(dto.UsernameOrEmail);
                }

                if (user == null)
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Invalid username or password");
                }

                // Verify password
                if (!PasswordHelper.VerifyPassword(dto.Password, user.PasswordHash))
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Invalid username or password");
                }

                // Check if user is active
                if (!user.IsActive)
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Account is deactivated. Please contact administrator.");
                }

                // Get user roles
                var roles = user.UserRoles.Select(ur => ur.Role.Name).ToList();

                // Generate tokens
                var accessToken = _tokenService.GenerateAccessToken(user, roles);
                var refreshToken = _tokenService.GenerateRefreshToken();

                // Save refresh token
                user.RefreshToken = refreshToken;
                user.RefreshTokenExpiryTime = DateTime.UtcNow.AddDays(7);
                user.LastLoginAt = DateTime.UtcNow;
                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                var userDto = new UserDto
                {
                    Id = user.Id,
                    Username = user.Username,
                    Email = user.Email,
                    FirstName = user.FirstName,
                    LastName = user.LastName,
                    PhoneNumber = user.PhoneNumber,
                    IsActive = user.IsActive,
                    EmailConfirmed = user.EmailConfirmed,
                    LastLoginAt = user.LastLoginAt,
                    Roles = roles,
                    EmployeeId = user.Employee?.Id
                };

                var response = new LoginResponseDto
                {
                    AccessToken = accessToken,
                    RefreshToken = refreshToken,
                    ExpiresAt = DateTime.UtcNow.AddMinutes(60),
                    User = userDto
                };

                return ResponseDto<LoginResponseDto>.SuccessResponse(response, "Login successful");
            }
            catch (Exception ex)
            {
                return ResponseDto<LoginResponseDto>.FailureResponse($"Error during login: {ex.Message}");
            }
        }

        public async Task<ResponseDto<LoginResponseDto>> RefreshTokenAsync(RefreshTokenDto dto)
        {
            try
            {
                // Validate access token
                var (isValid, username) = _tokenService.ValidateToken(dto.AccessToken);
                if (!isValid || username == null)
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Invalid access token");
                }

                // Find user by refresh token
                var user = await _unitOfWork.Users.GetByRefreshTokenAsync(dto.RefreshToken);
                if (user == null || user.Username != username)
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Invalid refresh token");
                }

                // Check if refresh token is expired
                if (user.RefreshTokenExpiryTime <= DateTime.UtcNow)
                {
                    return ResponseDto<LoginResponseDto>.FailureResponse("Refresh token has expired");
                }

                // Get user roles
                var roles = user.UserRoles.Select(ur => ur.Role.Name).ToList();

                // Generate new tokens
                var newAccessToken = _tokenService.GenerateAccessToken(user, roles);
                var newRefreshToken = _tokenService.GenerateRefreshToken();

                // Update refresh token
                user.RefreshToken = newRefreshToken;
                user.RefreshTokenExpiryTime = DateTime.UtcNow.AddDays(7);
                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                var userDto = new UserDto
                {
                    Id = user.Id,
                    Username = user.Username,
                    Email = user.Email,
                    FirstName = user.FirstName,
                    LastName = user.LastName,
                    PhoneNumber = user.PhoneNumber,
                    IsActive = user.IsActive,
                    EmailConfirmed = user.EmailConfirmed,
                    LastLoginAt = user.LastLoginAt,
                    Roles = roles,
                    EmployeeId = user.Employee?.Id
                };

                var response = new LoginResponseDto
                {
                    AccessToken = newAccessToken,
                    RefreshToken = newRefreshToken,
                    ExpiresAt = DateTime.UtcNow.AddMinutes(60),
                    User = userDto
                };

                return ResponseDto<LoginResponseDto>.SuccessResponse(response, "Token refreshed successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<LoginResponseDto>.FailureResponse($"Error refreshing token: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> RevokeTokenAsync(int userId)
        {
            try
            {
                var user = await _unitOfWork.Users.GetByIdAsync(userId);
                if (user == null)
                {
                    return ResponseDto<bool>.FailureResponse("User not found");
                }

                user.RefreshToken = null;
                user.RefreshTokenExpiryTime = null;
                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Token revoked successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error revoking token: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> ChangePasswordAsync(int userId, ChangePasswordDto dto)
        {
            try
            {
                if (dto.NewPassword != dto.ConfirmPassword)
                {
                    return ResponseDto<bool>.FailureResponse("New passwords do not match");
                }

                var user = await _unitOfWork.Users.GetByIdAsync(userId);
                if (user == null)
                {
                    return ResponseDto<bool>.FailureResponse("User not found");
                }

                // Verify current password
                if (!PasswordHelper.VerifyPassword(dto.CurrentPassword, user.PasswordHash))
                {
                    return ResponseDto<bool>.FailureResponse("Current password is incorrect");
                }

                // Update password
                user.PasswordHash = PasswordHelper.HashPassword(dto.NewPassword);
                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Password changed successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error changing password: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> RequestPasswordResetAsync(string email)
        {
            try
            {
                var user = await _unitOfWork.Users.GetByEmailAsync(email);
                if (user == null)
                {
                    // Don't reveal if email exists
                    return ResponseDto<bool>.SuccessResponse(true, "If the email exists, a password reset link has been sent");
                }

                // TODO: Generate reset token and send email
                // For now, just return success
                return ResponseDto<bool>.SuccessResponse(true, "Password reset link has been sent to your email");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error requesting password reset: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> ResetPasswordAsync(ResetPasswordDto dto)
        {
            try
            {
                if (dto.NewPassword != dto.ConfirmPassword)
                {
                    return ResponseDto<bool>.FailureResponse("Passwords do not match");
                }

                // TODO: Implement password reset token validation
                var user = await _unitOfWork.Users.GetByEmailAsync(dto.Email);
                if (user == null)
                {
                    return ResponseDto<bool>.FailureResponse("Invalid reset token");
                }

                user.PasswordHash = PasswordHelper.HashPassword(dto.NewPassword);
                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Password reset successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error resetting password: {ex.Message}");
            }
        }
    }
}