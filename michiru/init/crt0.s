; Fake crt0.s for init process
; Do nothing right now.

[global start]
[extern main]
start:
	call main
	jmp $
