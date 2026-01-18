using AutoMapper;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;
using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.LeaveRequest;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.Services.Implementation
{
    public class LeaveRequestService : ILeaveRequestService
    {
        private readonly IUnitOfWork _unitOfWork;
        private readonly IMapper _mapper;

        public LeaveRequestService(IUnitOfWork unitOfWork, IMapper mapper)
        {
            _unitOfWork = unitOfWork;
            _mapper = mapper;
        }

        public async Task<ResponseDto<LeaveRequestDto>> GetByIdAsync(int id)
        {
            try
            {
                var leaveRequest = await _unitOfWork.LeaveRequests.GetByIdAsync(id);
                if (leaveRequest == null)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Leave request not found");
                }

                var leaveRequestDto = _mapper.Map<LeaveRequestDto>(leaveRequest);
                return ResponseDto<LeaveRequestDto>.SuccessResponse(leaveRequestDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<LeaveRequestDto>.FailureResponse($"Error retrieving leave request: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<LeaveRequestDto>>> GetByEmployeeIdAsync(int employeeId)
        {
            try
            {
                var leaveRequests = await _unitOfWork.LeaveRequests.GetByEmployeeIdAsync(employeeId);
                var leaveRequestDtos = _mapper.Map<List<LeaveRequestDto>>(leaveRequests);
                return ResponseDto<List<LeaveRequestDto>>.SuccessResponse(leaveRequestDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<LeaveRequestDto>>.FailureResponse($"Error retrieving leave requests: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<LeaveRequestDto>>> GetPendingRequestsAsync()
        {
            try
            {
                var leaveRequests = await _unitOfWork.LeaveRequests.GetPendingRequestsAsync();
                var leaveRequestDtos = _mapper.Map<List<LeaveRequestDto>>(leaveRequests);
                return ResponseDto<List<LeaveRequestDto>>.SuccessResponse(leaveRequestDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<LeaveRequestDto>>.FailureResponse($"Error retrieving pending requests: {ex.Message}");
            }
        }

        public async Task<ResponseDto<LeaveRequestDto>> CreateAsync(CreateLeaveRequestDto dto)
        {
            try
            {
                // Check if employee exists
                var employee = await _unitOfWork.Employees.GetByIdAsync(dto.EmployeeId);
                if (employee == null)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Employee not found");
                }

                // Validate dates
                if (dto.EndDate < dto.StartDate)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("End date cannot be before start date");
                }

                var leaveRequest = _mapper.Map<Core.Entities.LeaveRequest>(dto);
                await _unitOfWork.LeaveRequests.AddAsync(leaveRequest);
                await _unitOfWork.SaveChangesAsync();

                // Reload to get employee data
                leaveRequest = await _unitOfWork.LeaveRequests.GetByIdAsync(leaveRequest.Id);
                var leaveRequestDto = _mapper.Map<LeaveRequestDto>(leaveRequest);
                return ResponseDto<LeaveRequestDto>.SuccessResponse(leaveRequestDto, "Leave request created successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<LeaveRequestDto>.FailureResponse($"Error creating leave request: {ex.Message}");
            }
        }

        public async Task<ResponseDto<LeaveRequestDto>> UpdateAsync(int id, UpdateLeaveRequestDto dto)
        {
            try
            {
                var leaveRequest = await _unitOfWork.LeaveRequests.GetByIdAsync(id);
                if (leaveRequest == null)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Leave request not found");
                }

                if (leaveRequest.Status != LeaveStatus.Pending)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Cannot update a leave request that has already been processed");
                }

                // Validate dates
                if (dto.EndDate < dto.StartDate)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("End date cannot be before start date");
                }

                _mapper.Map(dto, leaveRequest);
                await _unitOfWork.LeaveRequests.UpdateAsync(leaveRequest);
                await _unitOfWork.SaveChangesAsync();

                var leaveRequestDto = _mapper.Map<LeaveRequestDto>(leaveRequest);
                return ResponseDto<LeaveRequestDto>.SuccessResponse(leaveRequestDto, "Leave request updated successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<LeaveRequestDto>.FailureResponse($"Error updating leave request: {ex.Message}");
            }
        }

        public async Task<ResponseDto<LeaveRequestDto>> ApproveAsync(ApproveLeaveRequestDto dto)
        {
            try
            {
                var leaveRequest = await _unitOfWork.LeaveRequests.GetByIdAsync(dto.LeaveRequestId);
                if (leaveRequest == null)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Leave request not found");
                }

                if (leaveRequest.Status != LeaveStatus.Pending)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Leave request has already been processed");
                }

                leaveRequest.Status = LeaveStatus.Approved;
                leaveRequest.ApprovedBy = dto.ApprovedBy;
                leaveRequest.ApprovedAt = DateTime.UtcNow;

                await _unitOfWork.LeaveRequests.UpdateAsync(leaveRequest);
                await _unitOfWork.SaveChangesAsync();

                var leaveRequestDto = _mapper.Map<LeaveRequestDto>(leaveRequest);
                return ResponseDto<LeaveRequestDto>.SuccessResponse(leaveRequestDto, "Leave request approved successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<LeaveRequestDto>.FailureResponse($"Error approving leave request: {ex.Message}");
            }
        }

        public async Task<ResponseDto<LeaveRequestDto>> RejectAsync(RejectLeaveRequestDto dto)
        {
            try
            {
                var leaveRequest = await _unitOfWork.LeaveRequests.GetByIdAsync(dto.LeaveRequestId);
                if (leaveRequest == null)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Leave request not found");
                }

                if (leaveRequest.Status != LeaveStatus.Pending)
                {
                    return ResponseDto<LeaveRequestDto>.FailureResponse("Leave request has already been processed");
                }

                leaveRequest.Status = LeaveStatus.Rejected;
                leaveRequest.ApprovedBy = dto.RejectedBy;
                leaveRequest.ApprovedAt = DateTime.UtcNow;
                leaveRequest.RejectionReason = dto.RejectionReason;

                await _unitOfWork.LeaveRequests.UpdateAsync(leaveRequest);
                await _unitOfWork.SaveChangesAsync();

                var leaveRequestDto = _mapper.Map<LeaveRequestDto>(leaveRequest);
                return ResponseDto<LeaveRequestDto>.SuccessResponse(leaveRequestDto, "Leave request rejected");
            }
            catch (Exception ex)
            {
                return ResponseDto<LeaveRequestDto>.FailureResponse($"Error rejecting leave request: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> CancelAsync(int id)
        {
            try
            {
                var leaveRequest = await _unitOfWork.LeaveRequests.GetByIdAsync(id);
                if (leaveRequest == null)
                {
                    return ResponseDto<bool>.FailureResponse("Leave request not found");
                }

                if (leaveRequest.Status == LeaveStatus.Cancelled)
                {
                    return ResponseDto<bool>.FailureResponse("Leave request is already cancelled");
                }

                leaveRequest.Status = LeaveStatus.Cancelled;
                await _unitOfWork.LeaveRequests.UpdateAsync(leaveRequest);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Leave request cancelled successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error cancelling leave request: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> DeleteAsync(int id)
        {
            try
            {
                var leaveRequest = await _unitOfWork.LeaveRequests.GetByIdAsync(id);
                if (leaveRequest == null)
                {
                    return ResponseDto<bool>.FailureResponse("Leave request not found");
                }

                leaveRequest.IsDeleted = true;
                await _unitOfWork.LeaveRequests.UpdateAsync(leaveRequest);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Leave request deleted successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error deleting leave request: {ex.Message}");
            }
        }
    }
}