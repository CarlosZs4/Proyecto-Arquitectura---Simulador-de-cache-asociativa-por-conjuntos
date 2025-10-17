.data
array:          .space 40      # 10 elementos de 4 bytes = 40 bytes
tamano_array:   .word 10
repeticiones:   .word 3

.text
.globl main

main:
    # Inicializar registros
    li $s0, 0                   # r = 0 (repeticiones)
    lw $s1, repeticiones        # $s1 = repeticiones_max
    la $s2, array               # dirección base del array
    lw $s3, tamano_array        # $s3 = tamaño del array

loop_repeticiones_array:
    bge $s0, $s1, end_program_array
    
    li $t0, 0                   # i = 0
loop_array:
    bge $t0, $s3, end_loop_array
    
    # --- CÁLCULO DE DIRECCIÓN array[i] ---
    # dirección = array + i * 4
    
    sll $t1, $t0, 2             # $t1 = i * 4
    add $t2, $s2, $t1           # $t2 = array + offset
    
    # --- ACCESO A LA MEMORIA ---
    lw $t3, 0($t2)              # CARGAR array[i] en $t3
    
    # --- OPERACIÓN (simulada) ---
    addi $t3, $t3, 1            # incrementar valor
    sw $t3, 0($t2)              # ALMACENAR de vuelta en array[i]
    
    # --- INSTRUCCIONES DEL BUCLE ---
    addi $t0, $t0, 1            # i++
    
    j loop_array

end_loop_array:
    addi $s0, $s0, 1            # r++
    j loop_repeticiones_array

end_program_array:
    # Fin del programa
    li $v0, 10
    syscall
