EQUAL_ANY	    = 0000b
RANGES		    = 0100b
EQUAL_EACH	    = 1000b
EQUAL_ORDERED	    = 1100b
NEGATIVE_POLARITY = 010000b
BYTE_MASK	 = 1000000b

; ==== strcmp ====

strcmp_sse42:
  ; Using __fastcall convention, ecx = string1, edx = string2
  mov eax, ecx
  sub eax, edx ; eax = ecx - edx
  sub edx, 16

STRCMP_LOOP:
    add edx, 16
    MovDqU    xmm0, dqword[edx]
    ; find the first *different* bytes, hence negative polarity
    PcmpIstrI xmm0, dqword[edx + eax], EQUAL_EACH + NEGATIVE_POLARITY
    ja STRCMP_LOOP

  jc STRCMP_DIFF

  ; the strings are equal
  xor eax, eax
  ret
STRCMP_DIFF:
  ; subtract the first different bytes
  add eax, edx
  movzx eax, byte[eax + ecx]
  movzx edx, byte[edx + ecx]
  sub eax, edx
  ret


; ==== strlen ====
strlen_sse42:
  ; ecx = string
  mov eax, -16
  mov edx, ecx
  pxor xmm0, xmm0

STRLEN_LOOP:
    add eax, 16
    PcmpIstrI xmm0, dqword[edx + eax], EQUAL_EACH
    jnz STRLEN_LOOP

  add eax, ecx
  ret

; ==== strstr ====
strstr_sse42:
  ; ecx = haystack, edx = needle

  push esi
  push edi
  MovDqU xmm2, dqword[edx] ; load the first 16 bytes of neddle
  Pxor xmm3, xmm3
  lea eax, [ecx - 16]

  ; find the first possible match of 16-byte fragment in haystack
STRSTR_MAIN_LOOP:
    add eax, 16
    PcmpIstrI xmm2, dqword[eax], EQUAL_ORDERED
    ja STRSTR_MAIN_LOOP

  jnc STRSTR_NOT_FOUND

  add eax, ecx ; save the possible match start
  mov edi, edx
  mov esi, eax
  sub edi, esi
  sub esi, 16

  ; compare the strings
@@:
    add esi, 16
    MovDqU    xmm1, dqword[esi + edi]
    ; mask out invalid bytes in the haystack
    PcmpIstrM xmm3, xmm1, EQUAL_EACH + NEGATIVE_POLARITY + BYTE_MASK
    MovDqU xmm4, dqword[esi]
    PAnd xmm4, xmm0
    PcmpIstrI xmm1, xmm4, EQUAL_EACH + NEGATIVE_POLARITY
    ja @B

  jnc STRSTR_FOUND

  ; continue searching from the next byte
  sub eax, 15
  jmp STRSTR_MAIN_LOOP

STRSTR_NOT_FOUND:
  xor eax, eax

STRSTR_FOUND:
  pop edi
  pop esi
  ret
