
#include "trace.h"


char __trace_file[] = "C:\\temp\\trace.log";
int __trace = 1;

void trace(const wchar_t* format, ...)
{
	if (!__trace) return;

	FILE* f = stderr;

	if (__trace_file != nullptr)
	{
		if (fopen_s(&f, __trace_file, "a") != 0)
		{
			fprintf(stderr, "WARNING: Failed to open trace file '%s' for writing!\n", __trace_file);
			return;
		}
	}

	va_list ap;
	va_start(ap, format);
	vfwprintf(f, format, ap);
	va_end(ap);

	fprintf(f, "\n");

	if (__trace_file != nullptr) fclose(f);
}


//send log to output
#define OUTLOG(s)  \
{                   \
    std::ostringstream os;\
    os << s << std::endl;\
    OutputDebugStringA(os.str().c_str());   \
}

void printLog(LPCSTR s, ...)
{
	char buff[1024];
	int length = strlen(s);

	va_list ap;
	va_start(ap, s);
	_vsnprintf_s(buff, sizeof(buff), s, ap); //sizeof(szBuff)
	va_end(ap);

	OUTLOG(buff);
}