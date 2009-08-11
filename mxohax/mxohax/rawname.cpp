#pragma warning(disable:4273)

struct __type_info_node {
	void *memPtr;
	__type_info_node* next;
};
extern __type_info_node __type_info_root_node;

class type_info {
public:
	__declspec(dllimport) const char* name() const;
	__declspec(dllimport) const char* name(__type_info_node*) const;
};
const char* type_info::name() const {
	return name(&__type_info_root_node);
}