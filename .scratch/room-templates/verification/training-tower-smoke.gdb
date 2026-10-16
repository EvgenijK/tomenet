set pagination off
set confirm off
set debuginfod enabled off
break get_location
commands
silent
return
continue
end
break go_engine_init
commands
silent
return
continue
end
break setup_contact_socket
run -z
source .scratch/room-templates/verification/training-tower-smoke.py
