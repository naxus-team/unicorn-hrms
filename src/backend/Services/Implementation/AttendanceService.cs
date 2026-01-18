using AutoMapper;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;
using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.Attendance;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.Services.Implementation
{
    public class AttendanceService : IAttendanceService
    {
        private readonly IUnitOfWork _unitOfWork;
        private readonly IMapper _mapper;

        public AttendanceService(IUnitOfWork unitOfWork, IMapper mapper)
        {
            _unitOfWork = unitOfWork;
            _mapper = mapper;
        }

        public async Task<ResponseDto<AttendanceDto>> GetByIdAsync(int id)
        {
            try
            {
                var attendance = await _unitOfWork.Attendances.GetByIdAsync(id);
                if (attendance == null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Attendance record not found");
                }

                var attendanceDto = _mapper.Map<AttendanceDto>(attendance);
                return ResponseDto<AttendanceDto>.SuccessResponse(attendanceDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<AttendanceDto>.FailureResponse($"Error retrieving attendance: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<AttendanceDto>>> GetByEmployeeIdAsync(int employeeId, DateTime? startDate = null, DateTime? endDate = null)
        {
            try
            {
                var attendances = await _unitOfWork.Attendances.GetByEmployeeIdAsync(employeeId, startDate, endDate);
                var attendanceDtos = _mapper.Map<List<AttendanceDto>>(attendances);
                return ResponseDto<List<AttendanceDto>>.SuccessResponse(attendanceDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<AttendanceDto>>.FailureResponse($"Error retrieving attendances: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<AttendanceDto>>> GetByDateRangeAsync(DateTime startDate, DateTime endDate)
        {
            try
            {
                var attendances = await _unitOfWork.Attendances.GetByDateRangeAsync(startDate, endDate);
                var attendanceDtos = _mapper.Map<List<AttendanceDto>>(attendances);
                return ResponseDto<List<AttendanceDto>>.SuccessResponse(attendanceDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<AttendanceDto>>.FailureResponse($"Error retrieving attendances: {ex.Message}");
            }
        }

        public async Task<ResponseDto<AttendanceDto>> GetByEmployeeAndDateAsync(int employeeId, DateTime date)
        {
            try
            {
                var attendance = await _unitOfWork.Attendances.GetByEmployeeAndDateAsync(employeeId, date);
                if (attendance == null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Attendance record not found");
                }

                var attendanceDto = _mapper.Map<AttendanceDto>(attendance);
                return ResponseDto<AttendanceDto>.SuccessResponse(attendanceDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<AttendanceDto>.FailureResponse($"Error retrieving attendance: {ex.Message}");
            }
        }

        public async Task<ResponseDto<AttendanceDto>> CreateAsync(CreateAttendanceDto dto)
        {
            try
            {
                // Check if employee exists
                var employee = await _unitOfWork.Employees.GetByIdAsync(dto.EmployeeId);
                if (employee == null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Employee not found");
                }

                // Check if attendance already exists for this date
                var existing = await _unitOfWork.Attendances.GetByEmployeeAndDateAsync(dto.EmployeeId, dto.Date);
                if (existing != null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Attendance record already exists for this date");
                }

                var attendance = _mapper.Map<Core.Entities.Attendance>(dto);
                await _unitOfWork.Attendances.AddAsync(attendance);
                await _unitOfWork.SaveChangesAsync();

                // Reload to get employee data
                attendance = await _unitOfWork.Attendances.GetByIdAsync(attendance.Id);
                var attendanceDto = _mapper.Map<AttendanceDto>(attendance);
                return ResponseDto<AttendanceDto>.SuccessResponse(attendanceDto, "Attendance created successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<AttendanceDto>.FailureResponse($"Error creating attendance: {ex.Message}");
            }
        }

        public async Task<ResponseDto<AttendanceDto>> UpdateAsync(int id, UpdateAttendanceDto dto)
        {
            try
            {
                var attendance = await _unitOfWork.Attendances.GetByIdAsync(id);
                if (attendance == null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Attendance record not found");
                }

                _mapper.Map(dto, attendance);
                await _unitOfWork.Attendances.UpdateAsync(attendance);
                await _unitOfWork.SaveChangesAsync();

                var attendanceDto = _mapper.Map<AttendanceDto>(attendance);
                return ResponseDto<AttendanceDto>.SuccessResponse(attendanceDto, "Attendance updated successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<AttendanceDto>.FailureResponse($"Error updating attendance: {ex.Message}");
            }
        }

        public async Task<ResponseDto<AttendanceDto>> CheckInAsync(CheckInDto dto)
        {
            try
            {
                // Check if employee exists
                var employee = await _unitOfWork.Employees.GetByIdAsync(dto.EmployeeId);
                if (employee == null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Employee not found");
                }

                // Check if already checked in
                var existing = await _unitOfWork.Attendances.GetByEmployeeAndDateAsync(dto.EmployeeId, dto.Date);
                if (existing != null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Already checked in for today");
                }

                var attendance = new Core.Entities.Attendance
                {
                    EmployeeId = dto.EmployeeId,
                    Date = dto.Date.Date,
                    CheckIn = dto.CheckIn,
                    Status = AttendanceStatus.Present
                };

                await _unitOfWork.Attendances.AddAsync(attendance);
                await _unitOfWork.SaveChangesAsync();

                // Reload to get employee data
                attendance = await _unitOfWork.Attendances.GetByIdAsync(attendance.Id);
                var attendanceDto = _mapper.Map<AttendanceDto>(attendance);
                return ResponseDto<AttendanceDto>.SuccessResponse(attendanceDto, "Checked in successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<AttendanceDto>.FailureResponse($"Error checking in: {ex.Message}");
            }
        }

        public async Task<ResponseDto<AttendanceDto>> CheckOutAsync(CheckOutDto dto)
        {
            try
            {
                var attendance = await _unitOfWork.Attendances.GetByEmployeeAndDateAsync(dto.EmployeeId, dto.Date);
                if (attendance == null)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("No check-in record found for today");
                }
                if (attendance.CheckOut.HasValue)
                {
                    return ResponseDto<AttendanceDto>.FailureResponse("Already checked out");
                }
                attendance.CheckOut = dto.CheckOut;
                await _unitOfWork.Attendances.UpdateAsync(attendance);
                await _unitOfWork.SaveChangesAsync(); var attendanceDto = _mapper.Map<AttendanceDto>(attendance);
                return ResponseDto<AttendanceDto>.SuccessResponse(attendanceDto, "Checked out successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<AttendanceDto>.FailureResponse($"Error checking out: {ex.Message}");
            }
        }
        public async Task<ResponseDto<bool>> DeleteAsync(int id)
        {
            try
            {
                var attendance = await _unitOfWork.Attendances.GetByIdAsync(id);
                if (attendance == null)
                {
                    return ResponseDto<bool>.FailureResponse("Attendance record not found");
                }
                attendance.IsDeleted = true;
                await _unitOfWork.Attendances.UpdateAsync(attendance);
                await _unitOfWork.SaveChangesAsync(); return ResponseDto<bool>.SuccessResponse(true, "Attendance deleted successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error deleting attendance: {ex.Message}");
            }
        }
    }
}