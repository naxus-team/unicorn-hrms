using AutoMapper;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;
using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.User;
using UnicornHRMS.Services.Helpers;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.Services.Implementation
{
    public class UserService : IUserService
    {
        private readonly IUnitOfWork _unitOfWork;
        private readonly IMapper _mapper;

        public UserService(IUnitOfWork unitOfWork, IMapper mapper)
        {
            _unitOfWork = unitOfWork;
            _mapper = mapper;
        }

        public async Task<ResponseDto<UserDto>> GetByIdAsync(int id)
        {
            try
            {
                var user = await _unitOfWork.Users.GetUserWithRolesAsync(id);
                if (user == null)
                {
                    return ResponseDto<UserDto>.FailureResponse("User not found");
                }

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
                    Roles = user.UserRoles.Select(ur => ur.Role.Name).ToList(),
                    EmployeeId = user.Employee?.Id
                };

                return ResponseDto<UserDto>.SuccessResponse(userDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<UserDto>.FailureResponse($"Error retrieving user: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<UserDto>>> GetAllAsync()
        {
            try
            {
                var users = await _unitOfWork.Users.GetUsersWithRolesAsync();
                var userDtos = users.Select(u => new UserDto
                {
                    Id = u.Id,
                    Username = u.Username,
                    Email = u.Email,
                    FirstName = u.FirstName,
                    LastName = u.LastName,
                    PhoneNumber = u.PhoneNumber,
                    IsActive = u.IsActive,
                    EmailConfirmed = u.EmailConfirmed,
                    LastLoginAt = u.LastLoginAt,
                    Roles = u.UserRoles.Select(ur => ur.Role.Name).ToList(),
                    EmployeeId = u.Employee?.Id
                }).ToList();

                return ResponseDto<List<UserDto>>.SuccessResponse(userDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<UserDto>>.FailureResponse($"Error retrieving users: {ex.Message}");
            }
        }

        public async Task<ResponseDto<UserDto>> CreateAsync(CreateUserDto dto)
        {
            try
            {
                if (await _unitOfWork.Users.UsernameExistsAsync(dto.Username))
                {
                    return ResponseDto<UserDto>.FailureResponse("Username already exists");
                }

                if (await _unitOfWork.Users.EmailExistsAsync(dto.Email))
                {
                    return ResponseDto<UserDto>.FailureResponse("Email already exists");
                }

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

                // Assign roles
                foreach (var roleName in dto.Roles)
                {
                    var role = await _unitOfWork.Roles.GetByNameAsync(roleName);
                    if (role != null)
                    {
                        var userRole = new UserRole
                        {
                            UserId = user.Id,
                            RoleId = role.Id
                        };
                        await _unitOfWork.Users.AddAsync(user);
                    }
                }

                await _unitOfWork.SaveChangesAsync();

                // Reload user with roles
                user = await _unitOfWork.Users.GetUserWithRolesAsync(user.Id);

                var userDto = new UserDto
                {
                    Id = user!.Id,
                    Username = user.Username,
                    Email = user.Email,
                    FirstName = user.FirstName,
                    LastName = user.LastName,
                    PhoneNumber = user.PhoneNumber,
                    IsActive = user.IsActive,
                    EmailConfirmed = user.EmailConfirmed,
                    Roles = user.UserRoles.Select(ur => ur.Role.Name).ToList()
                };

                return ResponseDto<UserDto>.SuccessResponse(userDto, "User created successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<UserDto>.FailureResponse($"Error creating user: {ex.Message}");
            }
        }

        public async Task<ResponseDto<UserDto>> UpdateAsync(int id, UpdateUserDto dto)
        {
            try
            {
                var user = await _unitOfWork.Users.GetByIdAsync(id);
                if (user == null)
                {
                    return ResponseDto<UserDto>.FailureResponse("User not found");
                }

                if (user.Email != dto.Email && await _unitOfWork.Users.EmailExistsAsync(dto.Email))
                {
                    return ResponseDto<UserDto>.FailureResponse("Email already exists");
                }

                user.Email = dto.Email;
                user.FirstName = dto.FirstName;
                user.LastName = dto.LastName;
                user.PhoneNumber = dto.PhoneNumber;
                user.IsActive = dto.IsActive;

                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                // Reload user with roles
                user = await _unitOfWork.Users.GetUserWithRolesAsync(id);

                var userDto = new UserDto
                {
                    Id = user!.Id,
                    Username = user.Username,
                    Email = user.Email,
                    FirstName = user.FirstName,
                    LastName = user.LastName,
                    PhoneNumber = user.PhoneNumber,
                    IsActive = user.IsActive,
                    EmailConfirmed = user.EmailConfirmed,
                    LastLoginAt = user.LastLoginAt,
                    Roles = user.UserRoles.Select(ur => ur.Role.Name).ToList(),
                    EmployeeId = user.Employee?.Id
                };

                return ResponseDto<UserDto>.SuccessResponse(userDto, "User updated successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<UserDto>.FailureResponse($"Error updating user: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> DeleteAsync(int id)
        {
            try
            {
                var user = await _unitOfWork.Users.GetByIdAsync(id);
                if (user == null)
                {
                    return ResponseDto<bool>.FailureResponse("User not found");
                }

                user.IsDeleted = true;
                await _unitOfWork.Users.UpdateAsync(user);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "User deleted successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error deleting user: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> AssignRoleAsync(AssignRoleDto dto)
        {
            try
            {
                var user = await _unitOfWork.Users.GetByIdAsync(dto.UserId);
                if (user == null)
                {
                    return ResponseDto<bool>.FailureResponse("User not found");
                }

                var role = await _unitOfWork.Roles.GetByNameAsync(dto.RoleName);
                if (role == null)
                {
                    return ResponseDto<bool>.FailureResponse("Role not found");
                }

                // Check if user already has this role
                var existingUserRole = await _unitOfWork.Users.FindAsync(ur => ur.Id == user.Id);
                if (existingUserRole.Any())
                {
                    return ResponseDto<bool>.FailureResponse("User already has this role");
                }

                var userRole = new UserRole
                {
                    UserId = dto.UserId,
                    RoleId = role.Id
                };

                await _unitOfWork.Users.AddAsync(user);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Role assigned successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error assigning role: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> RemoveRoleAsync(AssignRoleDto dto)
        {
            try
            {
                var user = await _unitOfWork.Users.GetUserWithRolesAsync(dto.UserId);
                if (user == null)
                {
                    return ResponseDto<bool>.FailureResponse("User not found");
                }

                var role = await _unitOfWork.Roles.GetByNameAsync(dto.RoleName);
                if (role == null)
                {
                    return ResponseDto<bool>.FailureResponse("Role not found");
                }

                var userRole = user.UserRoles.FirstOrDefault(ur => ur.RoleId == role.Id);
                if (userRole == null)
                {
                    return ResponseDto<bool>.FailureResponse("User does not have this role");
                }

                userRole.IsDeleted = true;
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Role removed successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error removing role: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<string>>> GetUserRolesAsync(int userId)
        {
            try
            {
                var roles = await _unitOfWork.Roles.GetRolesByUserIdAsync(userId);
                var roleNames = roles.Select(r => r.Name).ToList();

                return ResponseDto<List<string>>.SuccessResponse(roleNames);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<string>>.FailureResponse($"Error retrieving user roles: {ex.Message}");
            }
        }
    }
}