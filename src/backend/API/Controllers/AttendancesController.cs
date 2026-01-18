using Microsoft.AspNetCore.Mvc;
using UnicornHRMS.Services.DTOs.Attendance;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.API.Controllers
{
    public class AttendancesController : BaseApiController
    {
        private readonly IAttendanceService _attendanceService;

        public AttendancesController(IAttendanceService attendanceService)
        {
            _attendanceService = attendanceService;
        }

        /// <summary>
        /// Get attendance by ID
        /// </summary>
        [HttpGet("{id}")]
        public async Task<IActionResult> GetById(int id)
        {
            var response = await _attendanceService.GetByIdAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get attendance records by employee ID
        /// </summary>
        [HttpGet("employee/{employeeId}")]
        public async Task<IActionResult> GetByEmployeeId(int employeeId, [FromQuery] DateTime? startDate = null, [FromQuery] DateTime? endDate = null)
        {
            var response = await _attendanceService.GetByEmployeeIdAsync(employeeId, startDate, endDate);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get attendance by employee and specific date
        /// </summary>
        [HttpGet("employee/{employeeId}/date/{date}")]
        public async Task<IActionResult> GetByEmployeeAndDate(int employeeId, DateTime date)
        {
            var response = await _attendanceService.GetByEmployeeAndDateAsync(employeeId, date);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get all attendance records within a date range
        /// </summary>
        [HttpGet("date-range")]
        public async Task<IActionResult> GetByDateRange([FromQuery] DateTime startDate, [FromQuery] DateTime endDate)
        {
            var response = await _attendanceService.GetByDateRangeAsync(startDate, endDate);
            return HandleResponse(response);
        }

        /// <summary>
        /// Create attendance record
        /// </summary>
        [HttpPost]
        public async Task<IActionResult> Create([FromBody] CreateAttendanceDto dto)
        {
            var response = await _attendanceService.CreateAsync(dto);
            if (response.Success)
            {
                return CreatedAtAction(nameof(GetById), new { id = response.Data?.Id }, response);
            }
            return HandleResponse(response);
        }

        /// <summary>
        /// Check in employee
        /// </summary>
        [HttpPost("check-in")]
        public async Task<IActionResult> CheckIn([FromBody] CheckInDto dto)
        {
            var response = await _attendanceService.CheckInAsync(dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Check out employee
        /// </summary>
        [HttpPost("check-out")]
        public async Task<IActionResult> CheckOut([FromBody] CheckOutDto dto)
        {
            var response = await _attendanceService.CheckOutAsync(dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Update attendance record
        /// </summary>
        [HttpPut("{id}")]
        public async Task<IActionResult> Update(int id, [FromBody] UpdateAttendanceDto dto)
        {
            var response = await _attendanceService.UpdateAsync(id, dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Delete attendance record
        /// </summary>
        [HttpDelete("{id}")]
        public async Task<IActionResult> Delete(int id)
        {
            var response = await _attendanceService.DeleteAsync(id);
            return HandleResponse(response);
        }
    }
}