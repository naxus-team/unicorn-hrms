using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using UnicornHRMS.Services.DTOs.User;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.API.Controllers
{
    [Authorize]
    public class UsersController : BaseApiController
    {
        private readonly IUserService _userService;

        public UsersController(IUserService userService)
        {
            _userService = userService;
        }

        /// <summary>
        /// Get all users
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin")]
        [HttpGet]
        public async Task<IActionResult> GetAll()
        {
            var response = await _userService.GetAllAsync();
            return HandleResponse(response);
        }

        /// <summary>
        /// Get user by ID
        /// </summary>
        [HttpGet("{id}")]
        public async Task<IActionResult> GetById(int id)
        {
            var response = await _userService.GetByIdAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Create a new user
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin")]
        [HttpPost]
        public async Task<IActionResult> Create([FromBody] CreateUserDto dto)
        {
            var response = await _userService.CreateAsync(dto);
            if (response.Success)
            {
                return CreatedAtAction(nameof(GetById), new { id = response.Data?.Id }, response);
            }
            return HandleResponse(response);
        }

        /// <summary>
        /// Update user
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin")]
        [HttpPut("{id}")]
        public async Task<IActionResult> Update(int id, [FromBody] UpdateUserDto dto)
        {
            var response = await _userService.UpdateAsync(id, dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Delete user
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin")]
        [HttpDelete("{id}")]
        public async Task<IActionResult> Delete(int id)
        {
            var response = await _userService.DeleteAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Assign role to user
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin")]
        [HttpPost("assign-role")]
        public async Task<IActionResult> AssignRole([FromBody] AssignRoleDto dto)
        {
            var response = await _userService.AssignRoleAsync(dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Remove role from user
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin")]
        [HttpPost("remove-role")]
        public async Task<IActionResult> RemoveRole([FromBody] AssignRoleDto dto)
        {
            var response = await _userService.RemoveRoleAsync(dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get user roles
        /// </summary>
        [HttpGet("{id}/roles")]
        public async Task<IActionResult> GetUserRoles(int id)
        {
            var response = await _userService.GetUserRolesAsync(id);
            return HandleResponse(response);
        }
    }
}