using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Services.DTOs.LeaveRequest
{
    public class LeaveRequestDto
    {
        public int Id { get; set; }
        public int EmployeeId { get; set; }
        public string EmployeeName { get; set; } = string.Empty;
        public DateTime StartDate { get; set; }
        public DateTime EndDate { get; set; }
        public string LeaveType { get; set; } = string.Empty;
        public string Reason { get; set; } = string.Empty;
        public string Status { get; set; } = string.Empty;
        public int? ApprovedBy { get; set; }
        public DateTime? ApprovedAt { get; set; }
        public string? RejectionReason { get; set; }
        public int TotalDays => (EndDate - StartDate).Days + 1;
        public DateTime CreatedAt { get; set; }
    }

    public class CreateLeaveRequestDto
    {
        public int EmployeeId { get; set; }
        public DateTime StartDate { get; set; }
        public DateTime EndDate { get; set; }
        public LeaveType LeaveType { get; set; }
        public string Reason { get; set; } = string.Empty;
    }

    public class UpdateLeaveRequestDto
    {
        public DateTime StartDate { get; set; }
        public DateTime EndDate { get; set; }
        public LeaveType LeaveType { get; set; }
        public string Reason { get; set; } = string.Empty;
    }

    public class ApproveLeaveRequestDto
    {
        public int LeaveRequestId { get; set; }
        public int ApprovedBy { get; set; }
    }

    public class RejectLeaveRequestDto
    {
        public int LeaveRequestId { get; set; }
        public int RejectedBy { get; set; }
        public string RejectionReason { get; set; } = string.Empty;
    }
}