using Microsoft.AspNetCore.Mvc;
using UnicornHRMS.Services.DTOs.Employee;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.API.Controllers
{
    public class EmployeesController : BaseApiController
    {
        private readonly IEmployeeService _employeeService;

        public EmployeesController(IEmployeeService employeeService)
        {
            _employeeService = employeeService;
        }

        /// <summary>
        /// Get all employees
        /// </summary>
        [HttpGet]
        public async Task<IActionResult> GetAll()
        {
            var response = await _employeeService.GetAllAsync();
            return HandleResponse(response);
        }

        /// <summary>
        /// Get active employees only
        /// </summary>
        [HttpGet("active")]
        public async Task<IActionResult> GetActive()
        {
            var response = await _employeeService.GetActiveEmployeesAsync();
            return HandleResponse(response);
        }

        /// <summary>
        /// Get employee by ID
        /// </summary>
        [HttpGet("{id}")]
        public async Task<IActionResult> GetById(int id)
        {
            var response = await _employeeService.GetByIdAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get employee by email
        /// </summary>
        [HttpGet("by-email/{email}")]
        public async Task<IActionResult> GetByEmail(string email)
        {
            var response = await _employeeService.GetByEmailAsync(email);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get employee by employee code
        /// </summary>
        [HttpGet("by-code/{code}")]
        public async Task<IActionResult> GetByCode(string code)
        {
            var response = await _employeeService.GetByEmployeeCodeAsync(code);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get employees by department
        /// </summary>
        [HttpGet("by-department/{department}")]
        public async Task<IActionResult> GetByDepartment(string department)
        {
            var response = await _employeeService.GetByDepartmentAsync(department);
            return HandleResponse(response);
        }

        /// <summary>
        /// Create a new employee
        /// </summary>
        [HttpPost]
        public async Task<IActionResult> Create([FromBody] CreateEmployeeDto dto)
        {
            var response = await _employeeService.CreateAsync(dto);
            if (response.Success)
            {
                return CreatedAtAction(nameof(GetById), new { id = response.Data?.Id }, response);
            }
            return HandleResponse(response);
        }

        /// <summary>
        /// Update an existing employee
        /// </summary>
        [HttpPut("{id}")]
        public async Task<IActionResult> Update(int id, [FromBody] UpdateEmployeeDto dto)
        {
            var response = await _employeeService.UpdateAsync(id, dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Deactivate an employee
        /// </summary>
        [HttpPatch("{id}/deactivate")]
        public async Task<IActionResult> Deactivate(int id)
        {
            var response = await _employeeService.DeactivateAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Activate an employee
        /// </summary>
        [HttpPatch("{id}/activate")]
        public async Task<IActionResult> Activate(int id)
        {
            var response = await _employeeService.ActivateAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Delete an employee (soft delete)
        /// </summary>
        [HttpDelete("{id}")]
        public async Task<IActionResult> Delete(int id)
        {
            var response = await _employeeService.DeleteAsync(id);
            return HandleResponse(response);
        }
    }
}