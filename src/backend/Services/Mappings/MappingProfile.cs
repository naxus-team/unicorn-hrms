using AutoMapper;
using UnicornHRMS.Core.Entities;
using UnicornHRMS.Services.DTOs.Employee;
using UnicornHRMS.Services.DTOs.Attendance;
using UnicornHRMS.Services.DTOs.LeaveRequest;
using UnicornHRMS.Services.DTOs.Department;
using UnicornHRMS.Services.DTOs.User;

namespace UnicornHRMS.Services.Mappings
{
    public class MappingProfile : Profile
    {
        public MappingProfile()
        {
            // Employee mappings
            CreateMap<Employee, EmployeeDto>();
            CreateMap<CreateEmployeeDto, Employee>()
                .ForMember(dest => dest.IsActive, opt => opt.MapFrom(src => true));
            CreateMap<UpdateEmployeeDto, Employee>()
                .ForMember(dest => dest.Id, opt => opt.Ignore())
                .ForMember(dest => dest.EmployeeCode, opt => opt.Ignore())
                .ForMember(dest => dest.CreatedAt, opt => opt.Ignore());

            // Attendance mappings
            CreateMap<Core.Entities.Attendance, AttendanceDto>()
                .ForMember(dest => dest.EmployeeName, opt => opt.MapFrom(src =>
                    src.Employee != null ? $"{src.Employee.FirstName} {src.Employee.LastName}" : string.Empty))
                .ForMember(dest => dest.Status, opt => opt.MapFrom(src => src.Status.ToString()));

            CreateMap<CreateAttendanceDto, Core.Entities.Attendance>();
            CreateMap<UpdateAttendanceDto, Core.Entities.Attendance>()
                .ForMember(dest => dest.Id, opt => opt.Ignore())
                .ForMember(dest => dest.EmployeeId, opt => opt.Ignore())
                .ForMember(dest => dest.Date, opt => opt.Ignore())
                .ForMember(dest => dest.CreatedAt, opt => opt.Ignore());

            // LeaveRequest mappings
            CreateMap<Core.Entities.LeaveRequest, LeaveRequestDto>()
                .ForMember(dest => dest.EmployeeName, opt => opt.MapFrom(src =>
                    src.Employee != null ? $"{src.Employee.FirstName} {src.Employee.LastName}" : string.Empty))
                .ForMember(dest => dest.LeaveType, opt => opt.MapFrom(src => src.LeaveType.ToString()))
                .ForMember(dest => dest.Status, opt => opt.MapFrom(src => src.Status.ToString()));

            CreateMap<CreateLeaveRequestDto, Core.Entities.LeaveRequest>()
                .ForMember(dest => dest.Status, opt => opt.MapFrom(src => LeaveStatus.Pending));

            CreateMap<UpdateLeaveRequestDto, Core.Entities.LeaveRequest>()
                .ForMember(dest => dest.Id, opt => opt.Ignore())
                .ForMember(dest => dest.EmployeeId, opt => opt.Ignore())
                .ForMember(dest => dest.Status, opt => opt.Ignore())
                .ForMember(dest => dest.CreatedAt, opt => opt.Ignore());

            // Department mappings
            CreateMap<Department, DepartmentDto>()
                .ForMember(dest => dest.ManagerName, opt => opt.MapFrom(src =>
                    src.Manager != null ? $"{src.Manager.FirstName} {src.Manager.LastName}" : null))
                .ForMember(dest => dest.ParentDepartmentName, opt => opt.MapFrom(src => src.ParentDepartment != null ? src.ParentDepartment.Name : null))
                .ForMember(dest => dest.EmployeeCount, opt => opt.MapFrom(src => src.Employees.Count));

            CreateMap<CreateDepartmentDto, Department>();
            CreateMap<UpdateDepartmentDto, Department>()
                .ForMember(dest => dest.Id, opt => opt.Ignore())
                .ForMember(dest => dest.Code, opt => opt.Ignore())
                .ForMember(dest => dest.CreatedAt, opt => opt.Ignore());

            // User mappings
            CreateMap<User, UserDto>()
                .ForMember(dest => dest.Roles, opt => opt.MapFrom(src => src.UserRoles.Select(ur => ur.Role.Name)))
                .ForMember(dest => dest.EmployeeId, opt => opt.MapFrom(src => src.Employee != null ? src.Employee.Id : (int?)null));
        }
    }
}