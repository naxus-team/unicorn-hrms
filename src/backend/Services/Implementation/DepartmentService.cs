using AutoMapper;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;
using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.Department;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.Services.Implementation
{
    public class DepartmentService : IDepartmentService
    {
        private readonly IUnitOfWork _unitOfWork;
        private readonly IMapper _mapper;

        public DepartmentService(IUnitOfWork unitOfWork, IMapper mapper)
        {
            _unitOfWork = unitOfWork;
            _mapper = mapper;
        }

        public async Task<ResponseDto<DepartmentDto>> GetByIdAsync(int id)
        {
            try
            {
                var department = await _unitOfWork.Departments.GetWithEmployeesAsync(id);
                if (department == null)
                {
                    return ResponseDto<DepartmentDto>.FailureResponse("Department not found");
                }

                var departmentDto = new DepartmentDto
                {
                    Id = department.Id,
                    Name = department.Name,
                    Code = department.Code,
                    Description = department.Description,
                    ManagerId = department.ManagerId,
                    ManagerName = department.Manager != null ? $"{department.Manager.FirstName} {department.Manager.LastName}" : null,
                    ParentDepartmentId = department.ParentDepartmentId,
                    ParentDepartmentName = department.ParentDepartment?.Name,
                    IsActive = department.IsActive,
                    EmployeeCount = department.Employees.Count,
                    CreatedAt = department.CreatedAt
                };

                return ResponseDto<DepartmentDto>.SuccessResponse(departmentDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<DepartmentDto>.FailureResponse($"Error retrieving department: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<DepartmentDto>>> GetAllAsync()
        {
            try
            {
                var departments = await _unitOfWork.Departments.GetAllAsync();
                var departmentDtos = departments.Select(d => new DepartmentDto
                {
                    Id = d.Id,
                    Name = d.Name,
                    Code = d.Code,
                    Description = d.Description,
                    ManagerId = d.ManagerId,
                    ParentDepartmentId = d.ParentDepartmentId,
                    IsActive = d.IsActive,
                    CreatedAt = d.CreatedAt
                }).ToList();

                return ResponseDto<List<DepartmentDto>>.SuccessResponse(departmentDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<DepartmentDto>>.FailureResponse($"Error retrieving departments: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<DepartmentDto>>> GetActiveDepartmentsAsync()
        {
            try
            {
                var departments = await _unitOfWork.Departments.GetActiveDepartmentsAsync();
                var departmentDtos = departments.Select(d => new DepartmentDto
                {
                    Id = d.Id,
                    Name = d.Name,
                    Code = d.Code,
                    Description = d.Description,
                    ManagerId = d.ManagerId,
                    ManagerName = d.Manager != null ? $"{d.Manager.FirstName} {d.Manager.LastName}" : null,
                    ParentDepartmentId = d.ParentDepartmentId,
                    ParentDepartmentName = d.ParentDepartment?.Name,
                    IsActive = d.IsActive,
                    CreatedAt = d.CreatedAt
                }).ToList();

                return ResponseDto<List<DepartmentDto>>.SuccessResponse(departmentDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<DepartmentDto>>.FailureResponse($"Error retrieving active departments: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<DepartmentDto>>> GetSubDepartmentsAsync(int parentId)
        {
            try
            {
                var departments = await _unitOfWork.Departments.GetSubDepartmentsAsync(parentId);
                var departmentDtos = departments.Select(d => new DepartmentDto
                {
                    Id = d.Id,
                    Name = d.Name,
                    Code = d.Code,
                    Description = d.Description,
                    ManagerId = d.ManagerId,
                    ManagerName = d.Manager != null ? $"{d.Manager.FirstName} {d.Manager.LastName}" : null,
                    ParentDepartmentId = d.ParentDepartmentId,
                    IsActive = d.IsActive,
                    CreatedAt = d.CreatedAt
                }).ToList();

                return ResponseDto<List<DepartmentDto>>.SuccessResponse(departmentDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<DepartmentDto>>.FailureResponse($"Error retrieving sub-departments: {ex.Message}");
            }
        }

        public async Task<ResponseDto<DepartmentDto>> GetByCodeAsync(string code)
        {
            try
            {
                var department = await _unitOfWork.Departments.GetByCodeAsync(code);
                if (department == null)
                {
                    return ResponseDto<DepartmentDto>.FailureResponse("Department not found");
                }

                var departmentDto = new DepartmentDto
                {
                    Id = department.Id,
                    Name = department.Name,
                    Code = department.Code,
                    Description = department.Description,
                    ManagerId = department.ManagerId,
                    ManagerName = department.Manager != null ? $"{department.Manager.FirstName} {department.Manager.LastName}" : null,
                    ParentDepartmentId = department.ParentDepartmentId,
                    ParentDepartmentName = department.ParentDepartment?.Name,
                    IsActive = department.IsActive,
                    CreatedAt = department.CreatedAt
                };

                return ResponseDto<DepartmentDto>.SuccessResponse(departmentDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<DepartmentDto>.FailureResponse($"Error retrieving department: {ex.Message}");
            }
        }

        public async Task<ResponseDto<DepartmentDto>> CreateAsync(CreateDepartmentDto dto)
        {
            try
            {
                // Check if code exists
                var existingDepartment = await _unitOfWork.Departments.GetByCodeAsync(dto.Code);
                if (existingDepartment != null)
                {
                    return ResponseDto<DepartmentDto>.FailureResponse("Department code already exists");
                }

                // Validate manager exists
                if (dto.ManagerId.HasValue)
                {
                    var manager = await _unitOfWork.Employees.GetByIdAsync(dto.ManagerId.Value);
                    if (manager == null)
                    {
                        return ResponseDto<DepartmentDto>.FailureResponse("Manager not found");
                    }
                }

                // Validate parent department exists
                if (dto.ParentDepartmentId.HasValue)
                {
                    var parentDepartment = await _unitOfWork.Departments.GetByIdAsync(dto.ParentDepartmentId.Value);
                    if (parentDepartment == null)
                    {
                        return ResponseDto<DepartmentDto>.FailureResponse("Parent department not found");
                    }
                }

                var department = new Department
                {
                    Name = dto.Name,
                    Code = dto.Code,
                    Description = dto.Description,
                    ManagerId = dto.ManagerId,
                    ParentDepartmentId = dto.ParentDepartmentId,
                    IsActive = true
                };

                await _unitOfWork.Departments.AddAsync(department);
                await _unitOfWork.SaveChangesAsync();

                // Reload with relationships
                department = await _unitOfWork.Departments.GetByIdAsync(department.Id);

                var departmentDto = new DepartmentDto
                {
                    Id = department!.Id,
                    Name = department.Name,
                    Code = department.Code,
                    Description = department.Description,
                    ManagerId = department.ManagerId,
                    ParentDepartmentId = department.ParentDepartmentId,
                    IsActive = department.IsActive,
                    CreatedAt = department.CreatedAt
                };

                return ResponseDto<DepartmentDto>.SuccessResponse(departmentDto, "Department created successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<DepartmentDto>.FailureResponse($"Error creating department: {ex.Message}");
            }
        }

        public async Task<ResponseDto<DepartmentDto>> UpdateAsync(int id, UpdateDepartmentDto dto)
        {
            try
            {
                var department = await _unitOfWork.Departments.GetByIdAsync(id);
                if (department == null)
                {
                    return ResponseDto<DepartmentDto>.FailureResponse("Department not found");
                }

                // Validate manager exists
                if (dto.ManagerId.HasValue)
                {
                    var manager = await _unitOfWork.Employees.GetByIdAsync(dto.ManagerId.Value);
                    if (manager == null)
                    {
                        return ResponseDto<DepartmentDto>.FailureResponse("Manager not found");
                    }
                }

                // Validate parent department exists and not circular reference
                if (dto.ParentDepartmentId.HasValue)
                {
                    if (dto.ParentDepartmentId.Value == id)
                    {
                        return ResponseDto<DepartmentDto>.FailureResponse("Department cannot be its own parent");
                    }

                    var parentDepartment = await _unitOfWork.Departments.GetByIdAsync(dto.ParentDepartmentId.Value);
                    if (parentDepartment == null)
                    {
                        return ResponseDto<DepartmentDto>.FailureResponse("Parent department not found");
                    }
                }

                department.Name = dto.Name;
                department.Description = dto.Description;
                department.ManagerId = dto.ManagerId;
                department.ParentDepartmentId = dto.ParentDepartmentId;
                department.IsActive = dto.IsActive;

                await _unitOfWork.Departments.UpdateAsync(department);
                await _unitOfWork.SaveChangesAsync();

                // Reload with relationships
                department = await _unitOfWork.Departments.GetByIdAsync(id);

                var departmentDto = new DepartmentDto
                {
                    Id = department!.Id,
                    Name = department.Name,
                    Code = department.Code,
                    Description = department.Description,
                    ManagerId = department.ManagerId,
                    ParentDepartmentId = department.ParentDepartmentId,
                    IsActive = department.IsActive,
                    CreatedAt = department.CreatedAt
                };

                return ResponseDto<DepartmentDto>.SuccessResponse(departmentDto, "Department updated successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<DepartmentDto>.FailureResponse($"Error updating department: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> DeleteAsync(int id)
        {
            try
            {
                var department = await _unitOfWork.Departments.GetWithEmployeesAsync(id);
                if (department == null)
                {
                    return ResponseDto<bool>.FailureResponse("Department not found");
                }

                // Check if department has employees
                if (department.Employees.Any())
                {
                    return ResponseDto<bool>.FailureResponse("Cannot delete department with employees. Please reassign employees first.");
                }

                // Check if department has sub-departments
                if (department.SubDepartments.Any())
                {
                    return ResponseDto<bool>.FailureResponse("Cannot delete department with sub-departments. Please delete sub-departments first.");
                }

                department.IsDeleted = true;
                await _unitOfWork.Departments.UpdateAsync(department);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Department deleted successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error deleting department: {ex.Message}");
            }
        }
    }
}