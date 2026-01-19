using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.Department;

namespace UnicornHRMS.Services.Interfaces
{
    public interface IDepartmentService
    {
        Task<ResponseDto<DepartmentDto>> GetByIdAsync(int id);
        Task<ResponseDto<List<DepartmentDto>>> GetAllAsync();
        Task<ResponseDto<List<DepartmentDto>>> GetActiveDepartmentsAsync();
        Task<ResponseDto<List<DepartmentDto>>> GetSubDepartmentsAsync(int parentId);
        Task<ResponseDto<DepartmentDto>> GetByCodeAsync(string code);
        Task<ResponseDto<DepartmentDto>> CreateAsync(CreateDepartmentDto dto);
        Task<ResponseDto<DepartmentDto>> UpdateAsync(int id, UpdateDepartmentDto dto);
        Task<ResponseDto<bool>> DeleteAsync(int id);
    }
}