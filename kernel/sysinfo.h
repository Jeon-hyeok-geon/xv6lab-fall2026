// 커널이 수집해 사용자 영역으로 복사할 시스템 상태의 공용 형식을 정의한다.
struct sysinfo {
  uint64 freepages;   // free list에 남아 있는 물리 페이지 수를 저장한다.
  uint64 nproc;       // UNUSED가 아닌 프로세스 슬롯 수를 저장한다.
};
