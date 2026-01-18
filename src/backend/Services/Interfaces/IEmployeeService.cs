using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.Employee;

namespace UnicornHRMS.Services.Interfaces
{
    public interface IEmployeeService
    {
        Task<ResponseDto<EmployeeDto>> GetByIdAsync(int id);
        Task<ResponseDto<List<EmployeeDto>>> GetAllAsync();
        Task<ResponseDto<List<EmployeeDto>>> GetActiveEmployeesAsync();
        Task<ResponseDto<List<EmployeeDto>>> GetByDepartmentAsync(string department);
        Task<ResponseDto<EmployeeDto>> GetByEmailAsync(string email);
        Task<ResponseDto<EmployeeDto>> GetByEmployeeCodeAsync(string employeeCode);
        Task<ResponseDto<EmployeeDto>> CreateAsync(CreateEmployeeDto dto);
        Task<ResponseDto<EmployeeDto>> UpdateAsync(int id, UpdateEmployeeDto dto);
        Task<ResponseDto<bool>> DeleteAsync(int id);
        Task<ResponseDto<bool>> DeactivateAsync(int id);
        Task<ResponseDto<bool>> ActivateAsync(int id);
    }
}