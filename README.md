# Pintos project
32비트 기반 minimal한 os를 구현하면서 운영체제의 역할 및 기능들을 일부 구현해본다.

구현해야하는 모듈은
* USERPORG
* THREAD
* VM
* FILESYS

이렇게 네개의 파트이며 순차적으로 프로젝트로 구현한다.

**해당 레포는 VM까지 구현되어 있으며, 113 testcase 모두 통과**

## Prj1 - UserProg 1
>ELF binary 파일을 핀토스에서 load후 user mode로 실행이 가능하도록 한다.

해당 구현이 가능하기 위해 아래의 구현순서로 개발한다.

1. Arguemnt Passing
2. Memory Access Validation
3. System call

### Arguemnt Passing
pintos가 부팅되고 옵션을 통해 ELF binary file을 copy하여 실행을 하도록 한다.

이때, command line에는 실행할 파일과 arguments들을 넘겨주는데, 이때, file_name과 arguments들을 분리하여 load시에 stack에 넘겨준다.
[commit](https://github.com/ljy2855/pintos/commit/1ef9f05e094f757e013ed3bf1edf5c2fdbabaec5)

### Memory Access Validation
user mode로 실행되고 있는 process는 virtual address로 여러 process가 모든 memory 영역을 사용하는 것처럼 virtualize를 진행한다.

프로세스가 virtual address상 kernel 영역에 접근을 막고, physical address로 mapping이 되지 않은 invalid한 access에 대한 접근을 막도록 구현한다. 

구현은 'threads/vaddr.h' , 'userprog/pagedir.h'을 참고한다.

### System Call
user mode에서 실행하는 system call은 stack에 arguments들을 쌓고 interrupt을 발생시킨다.

현재 process의 context를 intr_frame에 담아 kernel모드로 전환 후, syscall_handler를 호출한다. 기존 스켈레톤 코드에는 전 과정까지가 구현되어 있고 우리는 이후의 과정만 구현을 완료하면 된다.

syscall_handler 내부에서 stack의 esp로부터 argument들을 확인 후 system call number에 해당하는 구현을 완료후 return 값을 eax에 담아 user mode로 전환한다.

[prj1 submit](https://github.com/ljy2855/pintos/tree/0eef4d67e9ce0dd48ff9d7ef67910fe01e64d574)

## Prj2 - Thread 

## Prj3 - UserProg 2 

## Prj4 - VM 