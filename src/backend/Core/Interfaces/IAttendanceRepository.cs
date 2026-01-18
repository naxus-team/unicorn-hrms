using UnicornHRMS.Core.Entities;

namespace UnicornHRMS.Core.Interfaces
{
    public interface IAttendanceRepository : IRepository<Attendance>
    {
        Task<IEnumerable<Attendance>> GetByEmployeeIdAsync(int employeeId, DateTime? startDate = null, DateTime? endDate = null);
        Task<Attendance?> GetByEmployeeAndDateAsync(int employeeId, DateTime date);
        Task<IEnumerable<Attendance>> GetByDateRangeAsync(DateTime startDate, DateTime endDate);
    }
}