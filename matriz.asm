.data
matriz:         .space 256    # 8x8 elementos de 4 bytes = 256 bytes
size_element:   .word 4
filas:          .word 8
columnas:       .word 8

.text
.globl main

main:
    # Inicializar registros
    li $s0, 0                   # r = 0 (repeticiones)
    li $s1, 4                   # repeticiones_max = 4
    la $s2, matriz              # dirección base de la matriz
    lw $s3, filas               # $s3 = filas
    lw $s4, columnas            # $s4 = columnas

loop_repeticiones:
    bge $s0, $s1, end_repeticiones
    
    li $t0, 0                   # i = 0
loop_filas:
    bge $t0, $s3, end_filas
    
    li $t1, 0                   # j = 0
loop_columnas:
    bge $t1, $s4, end_columnas
    
    # --- CÁLCULO DE DIRECCIÓN matriz[i][j] ---
    # dirección = matriz + (i * columnas + j) * 4
    
    # i * columnas
    mul $t2, $t0, $s4           # $t2 = i * columnas
    
    # (i * columnas + j)
    add $t2, $t2, $t1           # $t2 = i * columnas + j
    
    # (i * columnas + j) * 4
    sll $t2, $t2, 2             # multiplicar por 4 (shift left 2)
    
    # dirección final
    add $t3, $s2, $t2           # $t3 = matriz + offset
    
    # --- ACCESO A LA MEMORIA ---
    lw $t4, 0($t3)              # CARGAR matriz[i][j] en $t4
    
    # --- OPERACIÓN (simulada) ---
    addi $t4, $t4, 1            # incrementar valor
    sw $t4, 0($t3)              # ALMACENAR de vuelta en matriz[i][j]
    
    addi $t1, $t1, 1            # j++
    j loop_columnas

end_columnas:
    addi $t0, $t0, 1            # i++
    j loop_filas

end_filas:
    addi $s0, $s0, 1            # r++
    j loop_repeticiones

end_repeticiones:
    # Fin del programa
    li $v0, 10
    syscall
