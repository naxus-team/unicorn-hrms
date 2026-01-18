using Microsoft.AspNetCore.Mvc;
using UnicornHRMS.Services.DTOs.LeaveRequest;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.API.Controllers
{
    public class LeaveRequestsController : BaseApiController
    {
        private readonly ILeaveRequestService _leaveRequestService;

        public LeaveRequestsController(ILeaveRequestService leaveRequestService)
        {
            _leaveRequestService = leaveRequestService;
        }

        /// <summary>
        /// Get leave request by ID
        /// </summary>
        [HttpGet("{id}")]
        public async Task<IActionResult> GetById(int id)
        {
            var response = await _leaveRequestService.GetByIdAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get leave requests by employee ID
        /// </summary>
        [HttpGet("employee/{employeeId}")]
        public async Task<IActionResult> GetByEmployeeId(int employeeId)
        {
            var response = await _leaveRequestService.GetByEmployeeIdAsync(employeeId);
            return HandleResponse(response);
        }

        /// <summary>
        /// Get all pending leave requests
        /// </summary>
        [HttpGet("pending")]
        public async Task<IActionResult> GetPending()
        {
            var response = await _leaveRequestService.GetPendingRequestsAsync();
            return HandleResponse(response);
        }

        /// <summary>
        /// Create a new leave request
        /// </summary>
        [HttpPost]
        public async Task<IActionResult> Create([FromBody] CreateLeaveRequestDto dto)
        {
            var response = await _leaveRequestService.CreateAsync(dto);
            if (response.Success)
            {
                return CreatedAtAction(nameof(GetById), new { id = response.Data?.Id }, response);
            }
            return HandleResponse(response);
        }

        /// <summary>
        /// Update leave request
        /// </summary>
        [HttpPut("{id}")]
        public async Task<IActionResult> Update(int id, [FromBody] UpdateLeaveRequestDto dto)
        {
            var response = await _leaveRequestService.UpdateAsync(id, dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Approve leave request
        /// </summary>
        [HttpPost("approve")]
        public async Task<IActionResult> Approve([FromBody] ApproveLeaveRequestDto dto)
        {
            var response = await _leaveRequestService.ApproveAsync(dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Reject leave request
        /// </summary>
        [HttpPost("reject")]
        public async Task<IActionResult> Reject([FromBody] RejectLeaveRequestDto dto)
        {
            var response = await _leaveRequestService.RejectAsync(dto);
            return HandleResponse(response);
        }

        /// <summary>
        /// Cancel leave request
        /// </summary>
        [HttpPatch("{id}/cancel")]
        public async Task<IActionResult> Cancel(int id)
        {
            var response = await _leaveRequestService.CancelAsync(id);
            return HandleResponse(response);
        }

        /// <summary>
        /// Delete leave request
        /// </summary>
        [HttpDelete("{id}")]
        public async Task<IActionResult> Delete(int id)
        {
            var response = await _leaveRequestService.DeleteAsync(id);
            return HandleResponse(response);
        }
    }
}