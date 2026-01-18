using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.LeaveRequest;

namespace UnicornHRMS.Services.Interfaces
{
    public interface ILeaveRequestService
    {
        Task<ResponseDto<LeaveRequestDto>> GetByIdAsync(int id);
        Task<ResponseDto<List<LeaveRequestDto>>> GetByEmployeeIdAsync(int employeeId);
        Task<ResponseDto<List<LeaveRequestDto>>> GetPendingRequestsAsync();
        Task<ResponseDto<LeaveRequestDto>> CreateAsync(CreateLeaveRequestDto dto);
        Task<ResponseDto<LeaveRequestDto>> UpdateAsync(int id, UpdateLeaveRequestDto dto);
        Task<ResponseDto<LeaveRequestDto>> ApproveAsync(ApproveLeaveRequestDto dto);
        Task<ResponseDto<LeaveRequestDto>> RejectAsync(RejectLeaveRequestDto dto);
        Task<ResponseDto<bool>> CancelAsync(int id);
        Task<ResponseDto<bool>> DeleteAsync(int id);
    }
}