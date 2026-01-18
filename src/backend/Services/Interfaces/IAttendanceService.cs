using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.Attendance;

namespace UnicornHRMS.Services.Interfaces
{
    public interface IAttendanceService
    {
        Task<ResponseDto<AttendanceDto>> GetByIdAsync(int id);
        Task<ResponseDto<List<AttendanceDto>>> GetByEmployeeIdAsync(int employeeId, DateTime? startDate = null, DateTime? endDate = null);
        Task<ResponseDto<List<AttendanceDto>>> GetByDateRangeAsync(DateTime startDate, DateTime endDate);
        Task<ResponseDto<AttendanceDto>> GetByEmployeeAndDateAsync(int employeeId, DateTime date);
        Task<ResponseDto<AttendanceDto>> CreateAsync(CreateAttendanceDto dto);
        Task<ResponseDto<AttendanceDto>> UpdateAsync(int id, UpdateAttendanceDto dto);
        Task<ResponseDto<AttendanceDto>> CheckInAsync(CheckInDto dto);
        Task<ResponseDto<AttendanceDto>> CheckOutAsync(CheckOutDto dto);
        Task<ResponseDto<bool>> DeleteAsync(int id);
    }
}