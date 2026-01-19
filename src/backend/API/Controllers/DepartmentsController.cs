using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using UnicornHRMS.Services.DTOs.Department;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.API.Controllers
{
    [Authorize]
    public class DepartmentsController : BaseApiController
    {
        private readonly IDepartmentService _departmentService;

        public DepartmentsController(IDepartmentService departmentService)
        {
            _departmentService = departmentService;
        }

        /// <summary>
        /// Get all departments
        /// </summary>
        [HttpGet]
        public async Task<IActionResult> GetAll()
        {
            var response = await _departmentService.GetAllAsync();
            return HandleResponse(response);
        }

        /// <summary>
        /// Get active departments only
        /// </summary>
        [HttpGet("active")]
        public async Task<IActionResult> GetActive()
        {
            var response = await _departmentService.GetActiveDepartmentsAsync();
            return HandleResponse(response);
        }

        /// <summary>
        /// Get department by ID
        /// </summary>
        [HttpGet("{id}")]
        public async Task<IActionResult> GetById(int id)
        {
            var response = await _departmentService.GetByIdAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get department by code
        /// </summary>
        [HttpGet("by-code/{code}")]
        public async Task<IActionResult> GetByCode(string code)
        {
            var response = await _departmentService.GetByCodeAsync(code);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get sub-departments
        /// </summary>
        [HttpGet("{parentId}/sub-departments")]
        public async Task<IActionResult> GetSubDepartments(int parentId)
        {
            var response = await _departmentService.GetSubDepartmentsAsync(parentId);
            return HandleResponse(response);
        }

        /// <summary>
        /// Create a new department
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin,HR")]
        [HttpPost]
        public async Task<IActionResult> Create([FromBody] CreateDepartmentDto dto)
        {
            var response = await _departmentService.CreateAsync(dto);
            if (response.Success)
            {
                return CreatedAtAction(nameof(GetById), new { id = response.Data?.Id }, response);
            }
            return HandleResponse(response);
        }

        /// <summary>
        /// Update department
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin,HR")]
        [HttpPut("{id}")]
        public async Task<IActionResult> Update(int id, [FromBody] UpdateDepartmentDto dto)
        {
            var response = await _departmentService.UpdateAsync(id, dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Delete department
        /// </summary>
        [Authorize(Roles = "Admin,SuperAdmin")]
        [HttpDelete("{id}")]
        public async Task<IActionResult> Delete(int id)
        {
            var response = await _departmentService.DeleteAsync(id);
            return HandleResponse(response);
        }
    }
}