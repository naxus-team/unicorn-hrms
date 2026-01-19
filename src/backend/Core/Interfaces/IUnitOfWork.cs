namespace UnicornHRMS.Core.Interfaces
{
    public interface IUnitOfWork : IDisposable
    {
        IEmployeeRepository Employees { get; }
        IAttendanceRepository Attendances { get; }
        ILeaveRequestRepository LeaveRequests { get; }
        IUserRepository Users { get; }
        IRoleRepository Roles { get; }
        IDepartmentRepository Departments { get; }

        Task<int> SaveChangesAsync();
        Task BeginTransactionAsync();
        Task CommitTransactionAsync();
        Task RollbackTransactionAsync();
    }
}