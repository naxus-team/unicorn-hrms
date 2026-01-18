using AutoMapper;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Core.Interfaces;
using UnicornHRMS.Services.DTOs.Common;
using UnicornHRMS.Services.DTOs.Employee;
using UnicornHRMS.Services.Interfaces;

namespace UnicornHRMS.Services.Implementation
{
    public class EmployeeService : IEmployeeService
    {
        private readonly IUnitOfWork _unitOfWork;
        private readonly IMapper _mapper;

        public EmployeeService(IUnitOfWork unitOfWork, IMapper mapper)
        {
            _unitOfWork = unitOfWork;
            _mapper = mapper;
        }

        public async Task<ResponseDto<EmployeeDto>> GetByIdAsync(int id)
        {
            try
            {
                var employee = await _unitOfWork.Employees.GetByIdAsync(id);
                if (employee == null)
                {
                    return ResponseDto<EmployeeDto>.FailureResponse("Employee not found");
                }

                var employeeDto = _mapper.Map<EmployeeDto>(employee);
                return ResponseDto<EmployeeDto>.SuccessResponse(employeeDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<EmployeeDto>.FailureResponse($"Error retrieving employee: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<EmployeeDto>>> GetAllAsync()
        {
            try
            {
                var employees = await _unitOfWork.Employees.GetAllAsync();
                var employeeDtos = _mapper.Map<List<EmployeeDto>>(employees);
                return ResponseDto<List<EmployeeDto>>.SuccessResponse(employeeDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<EmployeeDto>>.FailureResponse($"Error retrieving employees: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<EmployeeDto>>> GetActiveEmployeesAsync()
        {
            try
            {
                var employees = await _unitOfWork.Employees.GetActiveEmployeesAsync();
                var employeeDtos = _mapper.Map<List<EmployeeDto>>(employees);
                return ResponseDto<List<EmployeeDto>>.SuccessResponse(employeeDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<EmployeeDto>>.FailureResponse($"Error retrieving active employees: {ex.Message}");
            }
        }

        public async Task<ResponseDto<List<EmployeeDto>>> GetByDepartmentAsync(string department)
        {
            try
            {
                var employees = await _unitOfWork.Employees.GetByDepartmentAsync(department);
                var employeeDtos = _mapper.Map<List<EmployeeDto>>(employees);
                return ResponseDto<List<EmployeeDto>>.SuccessResponse(employeeDtos);
            }
            catch (Exception ex)
            {
                return ResponseDto<List<EmployeeDto>>.FailureResponse($"Error retrieving employees by department: {ex.Message}");
            }
        }

        public async Task<ResponseDto<EmployeeDto>> GetByEmailAsync(string email)
        {
            try
            {
                var employee = await _unitOfWork.Employees.GetByEmailAsync(email);
                if (employee == null)
                {
                    return ResponseDto<EmployeeDto>.FailureResponse("Employee not found");
                }

                var employeeDto = _mapper.Map<EmployeeDto>(employee);
                return ResponseDto<EmployeeDto>.SuccessResponse(employeeDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<EmployeeDto>.FailureResponse($"Error retrieving employee: {ex.Message}");
            }
        }

        public async Task<ResponseDto<EmployeeDto>> GetByEmployeeCodeAsync(string employeeCode)
        {
            try
            {
                var employee = await _unitOfWork.Employees.GetByEmployeeCodeAsync(employeeCode);
                if (employee == null)
                {
                    return ResponseDto<EmployeeDto>.FailureResponse("Employee not found");
                }

                var employeeDto = _mapper.Map<EmployeeDto>(employee);
                return ResponseDto<EmployeeDto>.SuccessResponse(employeeDto);
            }
            catch (Exception ex)
            {
                return ResponseDto<EmployeeDto>.FailureResponse($"Error retrieving employee: {ex.Message}");
            }
        }

        public async Task<ResponseDto<EmployeeDto>> CreateAsync(CreateEmployeeDto dto)
        {
            try
            {
                // Check if employee code already exists
                var existingByCode = await _unitOfWork.Employees.GetByEmployeeCodeAsync(dto.EmployeeCode);
                if (existingByCode != null)
                {
                    return ResponseDto<EmployeeDto>.FailureResponse("Employee code already exists");
                }

                // Check if email already exists
                var existingByEmail = await _unitOfWork.Employees.GetByEmailAsync(dto.Email);
                if (existingByEmail != null)
                {
                    return ResponseDto<EmployeeDto>.FailureResponse("Email already exists");
                }

                var employee = _mapper.Map<Employee>(dto);
                await _unitOfWork.Employees.AddAsync(employee);
                await _unitOfWork.SaveChangesAsync();

                var employeeDto = _mapper.Map<EmployeeDto>(employee);
                return ResponseDto<EmployeeDto>.SuccessResponse(employeeDto, "Employee created successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<EmployeeDto>.FailureResponse($"Error creating employee: {ex.Message}");
            }
        }

        public async Task<ResponseDto<EmployeeDto>> UpdateAsync(int id, UpdateEmployeeDto dto)
        {
            try
            {
                var employee = await _unitOfWork.Employees.GetByIdAsync(id);
                if (employee == null)
                {
                    return ResponseDto<EmployeeDto>.FailureResponse("Employee not found");
                }

                // Check if email is being changed and if it already exists
                if (employee.Email != dto.Email)
                {
                    var existingByEmail = await _unitOfWork.Employees.GetByEmailAsync(dto.Email);
                    if (existingByEmail != null)
                    {
                        return ResponseDto<EmployeeDto>.FailureResponse("Email already exists");
                    }
                }

                _mapper.Map(dto, employee);
                await _unitOfWork.Employees.UpdateAsync(employee);
                await _unitOfWork.SaveChangesAsync();

                var employeeDto = _mapper.Map<EmployeeDto>(employee);
                return ResponseDto<EmployeeDto>.SuccessResponse(employeeDto, "Employee updated successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<EmployeeDto>.FailureResponse($"Error updating employee: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> DeleteAsync(int id)
        {
            try
            {
                var employee = await _unitOfWork.Employees.GetByIdAsync(id);
                if (employee == null)
                {
                    return ResponseDto<bool>.FailureResponse("Employee not found");
                }

                employee.IsDeleted = true;
                await _unitOfWork.Employees.UpdateAsync(employee);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Employee deleted successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error deleting employee: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> DeactivateAsync(int id)
        {
            try
            {
                var employee = await _unitOfWork.Employees.GetByIdAsync(id);
                if (employee == null)
                {
                    return ResponseDto<bool>.FailureResponse("Employee not found");
                }

                employee.IsActive = false;
                await _unitOfWork.Employees.UpdateAsync(employee);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Employee deactivated successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error deactivating employee: {ex.Message}");
            }
        }

        public async Task<ResponseDto<bool>> ActivateAsync(int id)
        {
            try
            {
                var employee = await _unitOfWork.Employees.GetByIdAsync(id);
                if (employee == null)
                {
                    return ResponseDto<bool>.FailureResponse("Employee not found");
                }

                employee.IsActive = true;
                await _unitOfWork.Employees.UpdateAsync(employee);
                await _unitOfWork.SaveChangesAsync();

                return ResponseDto<bool>.SuccessResponse(true, "Employee activated successfully");
            }
            catch (Exception ex)
            {
                return ResponseDto<bool>.FailureResponse($"Error activating employee: {ex.Message}");
            }
        }
    }
}