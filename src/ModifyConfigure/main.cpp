#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <assert.h>
#pragma warning(disable : 4267 4244 4296)

#define MAX_LENGTH  (1024*4)


const char* modual_name()
{
	static char s_modual_name[MAX_LENGTH] = "";
	char szPath[MAX_LENGTH] = "";
	::GetModuleFileName(NULL, szPath, sizeof(szPath));
	char* p =  ::strrchr(szPath, '\\');
	strcpy(s_modual_name, p+1);
	return s_modual_name;
}

int usage(){
	printf("Modify failed\n");
	printf("%s configure_file_path key value\n", modual_name());
	return 0;
}

char* trim_left(char* value)
{
	assert(value);
	int i=0;
	int nLen = strlen(value);
	for (i=0; i<nLen; i++){
		if (value[i] != ' ') break;
	}
	if (i != 0){
		memmove(value, value+i, nLen-i);
		value[nLen-i] = 0;
	}
	return value;
}

char* trim_right(char* value)
{
	assert(value);
	int nLen = strlen(value);
	for (int i=nLen-1; i>=0; i--){
		if (value[i] == ' ') value[i] = 0;
		else break;
	}
	return value;
}

int file_length(FILE *f)
{
	assert(f);
	int pos = 0;
	int end = 0;
	if (f){
		pos = ftell (f);
		fseek (f, 0, SEEK_END);
		end = ftell (f);
		fseek (f, pos, SEEK_SET);
	}

	return end;
}

int main(int argc, char* argv[])
{
	if (argc < 4){
		return usage();
	}else{
		char pConfigureFile[MAX_LENGTH];
		char pKey[MAX_LENGTH]         ; 
		char pValue[MAX_LENGTH]        ; 

		strcpy(pConfigureFile, argv[1]);
		strcpy(pKey, argv[2]);
		strcpy(pValue, argv[3]);

		trim_left(trim_right(pConfigureFile));
		trim_left(trim_right(pKey));
		trim_left(trim_right(pValue));

		if (strlen(pConfigureFile) > 0 && strlen(pKey) > 0 && strlen(pValue) > 0){
			FILE* fp = fopen(pConfigureFile, "rb");
			if (fp){
				int file_size = file_length(fp);
				char* buffer_src = new char[file_size+MAX_LENGTH];
				char* buffer_dst = new char[file_size+MAX_LENGTH];
				int n_dst_size = 0;
				memset(buffer_src, 0, file_size+MAX_LENGTH);
				memset(buffer_dst, 0, file_size+MAX_LENGTH);

				fread(buffer_src, 1, file_size, fp);
				fclose(fp);
				fp = NULL;

				if (buffer_src[file_size-2] == '\r' && buffer_src[file_size-1] == '\n'){
				}else{
					buffer_src[file_size]   = '\r';
					buffer_src[file_size+1] = '\n';
					file_size += 2;
				}

				char szLine[MAX_LENGTH]="";
				char search_key[1024] = "";
				sprintf(search_key, "%s =" , pKey);
				char* p0 = buffer_src;
				char* p1 = NULL;	
				bool is_comment = false;
				for (;;){
					is_comment = false;
					p0 = strstr(p0, search_key);
					if (!p0) break;
					for (p1=p0-1; p1>=buffer_src; p1--){
						if (p1[0] == '\r' && p1[1] == '\n'){
							is_comment = false;
							break;
						}else if (p1[0] == '#'){
							is_comment = true;
							break;
						}
					}
					if (is_comment){
						p0 += strlen(search_key);
					}else{
						break;
					}
				}
		
				if (p0){
					memcpy(buffer_dst, buffer_src, p0 - buffer_src);
					n_dst_size += p0 - buffer_src;
					p1 = strstr(p0, "\r\n");
					sprintf(szLine, "%s = %s", pKey, pValue);
					memcpy(buffer_dst + n_dst_size, szLine, strlen(szLine));
					n_dst_size += strlen(szLine);
					memcpy(buffer_dst + n_dst_size, p1, buffer_src+file_size-p1);
					n_dst_size += buffer_src+file_size-p1;					
				}else{
					memcpy(buffer_dst, buffer_src, file_size);
					n_dst_size = file_size;
					sprintf(szLine, "%s = %s", pKey, pValue);
					memcpy(buffer_dst+n_dst_size, szLine, strlen(szLine));
					n_dst_size += strlen(szLine);
					buffer_dst[n_dst_size] = '\r';
					buffer_dst[n_dst_size+1] = '\n';
					n_dst_size += 2;				
				}

				fp = fopen(pConfigureFile, "wb");
				if (fp){
					fwrite(buffer_dst, 1, n_dst_size, fp);
					fclose(fp);
				}

				if (buffer_src) delete buffer_src;
				if (buffer_dst) delete buffer_dst;

				printf("Modify successed, %s %s = %s\n", pConfigureFile, pKey, pValue);

				return 0;
			}else{
				printf("%s is not existed\n", pConfigureFile);
				return usage();
			}
		}else{
			return usage();
		}
	}

	return 0;
}
