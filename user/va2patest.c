#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int global = 42;                      // 초기값이 있는 전역 변수이므로 데이터 영역에 놓인다

void
show(char *name, void *va)
{
  // va는 현재 프로세스 주소 공간의 가상 주소다.
  // va2pa()가 페이지 테이블을 따라가 대응하는 물리 주소를 돌려준다.
  // 변환에 실패하면 va2pa()는 0을 돌려준다.
  printf("%s  va %p -> pa %p\n", name, va, (void *)va2pa((uint64)va));
}

int
main(void)
{
  int local = 7;                      // 함수 안의 지역 변수이므로 스택에 놓인다

  // main 함수의 주소: 함수의 기계어가 놓인 코드(text) 영역의 가상 주소
  show("text ", (void *)main);
  // 전역 변수의 주소: 초기값이 있는 global이 놓인 데이터 영역의 가상 주소
  show("data ", &global);
  // 지역 변수의 주소: 현재 main 호출의 스택 프레임 안에 있는 가상 주소
  show("stack", &local);

  exit(0);                            // 상태 코드 0으로 프로세스를 정상 종료한다
}
