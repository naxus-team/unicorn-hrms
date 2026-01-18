namespace UnicornHRMS.Services.DTOs.Common
{
    public class ResponseDto<T>
    {
        public bool Success { get; set; }
        public string Message { get; set; } = string.Empty;
        public T? Data { get; set; }
        public List<string> Errors { get; set; } = new();

        public static ResponseDto<T> SuccessResponse(T data, string message = "Operation successful")
        {
            return new ResponseDto<T>
            {
                Success = true,
                Message = message,
                Data = data
            };
        }

        public static ResponseDto<T> FailureResponse(string message, List<string>? errors = null)
        {
            return new ResponseDto<T>
            {
                Success = false,
                Message = message,
                Errors = errors ?? new List<string>()
            };
        }
    }

    public class PagedResponseDto<T>
    {
        public List<T> Items { get; set; } = new();
        public int TotalCount { get; set; }
        public int PageNumber { get; set; }
        public int PageSize { get; set; }
        public int TotalPages => (int)Math.Ceiling(TotalCount / (double)PageSize);
        public bool HasPreviousPage => PageNumber > 1;
        public bool HasNextPage => PageNumber < TotalPages;
    }
}