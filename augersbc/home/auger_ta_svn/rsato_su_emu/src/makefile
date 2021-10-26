all: ../bin/su_bsu ../bin/su_emu ../bin/cdas_su_emu ../bin/cl

../bin/su_bsu: su_bsu_emu.c ub_side.c pm_side.c protocol.c
	gcc su_bsu_emu.c ub_side.c pm_side.c protocol.c -o ../bin/su_bsu

../bin/su_emu: su_emu.c ub_side.c protocol.c;\
	gcc su_emu.c ub_side.c protocol.c -o ../bin/su_emu

../bin/cdas_su_emu: cdas_su_emu.c msg_interpreter.c ub_side.c protocol.c t2.c t3req.c sockets.c
	gcc -g cdas_su_emu.c msg_interpreter.c ub_side.c protocol.c t2.c t3req.c sockets.c -o ../bin/cdas_su_emu


../bin/cl: cl.c sockets.c
	gcc cl.c sockets.c -o ../bin/cl



clean:;
	rm ../bin/su_bsu ../bin/su_emu ../bin/cdas_su_emu ../bin/cl
