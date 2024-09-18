#if 0
;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; Albedo                   0   xyzw        0     NONE   float   xyzw
; Emission                 0   xyzw        1     NONE   float   xyzw
; Normal                   0   xyz         2     NONE   float   xyz 
; WorldPosition            0   xyz         3     NONE   float       
; SV_Position              0   xyzw        4      POS   float       
;
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; SV_Target                0   xyzw        0   TARGET   float   xyzw
;
; shader hash: 194400a83148ea2b1b3a917692258ad1
;
; Pipeline Runtime Information: 
;
; Pixel Shader
; DepthOutput=0
; SampleFrequency=0
;
;
; Input signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; Albedo                   0                 linear       
; Emission                 0                 linear       
; Normal                   0                 linear       
; WorldPosition            0                 linear       
; SV_Position              0          noperspective       
;
; Output signature:
;
; Name                 Index             InterpMode DynIdx
; -------------------- ----- ---------------------- ------
; SV_Target                0                              
;
; Buffer Definitions:
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
;
;
; ViewId state:
;
; Number of inputs: 20, outputs: 4
; Outputs dependent on ViewId: {  }
; Inputs contributing to computation of Outputs:
;   output 0 depends on inputs: { 0, 4, 8, 9, 10 }
;   output 1 depends on inputs: { 1, 5, 8, 9, 10 }
;   output 2 depends on inputs: { 2, 6, 8, 9, 10 }
;   output 3 depends on inputs: { 3, 7, 8, 9, 10 }
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

define void @main() {
  %1 = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %2 = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %3 = call float @dx.op.loadInput.f32(i32 4, i32 2, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %4 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %5 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %6 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %7 = call float @dx.op.loadInput.f32(i32 4, i32 1, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %8 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %9 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %10 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %11 = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 3, i32 undef)  ; LoadInput(inputSigId,rowIndex,colIndex,gsVertexAxis)
  %12 = call float @dx.op.dot3.f32(i32 55, float %1, float %2, float %3, float %1, float %2, float %3)  ; Dot3(ax,ay,az,bx,by,bz)
  %13 = call float @dx.op.unary.f32(i32 25, float %12)  ; Rsqrt(value)
  %14 = fmul fast float %13, %1
  %15 = fmul fast float %13, %2
  %16 = fmul fast float %13, %3
  %17 = call float @dx.op.dot3.f32(i32 55, float %14, float %15, float %16, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00)  ; Dot3(ax,ay,az,bx,by,bz)
  %18 = fmul fast float %17, 0x3FD45F3080000000
  %19 = call float @dx.op.binary.f32(i32 35, float 0x3FA99999A0000000, float %18)  ; FMax(a,b)
  %20 = fmul fast float %19, %8
  %21 = fmul fast float %19, %9
  %22 = fmul fast float %19, %10
  %23 = fmul fast float %19, %11
  %24 = fadd fast float %20, %4
  %25 = fadd fast float %21, %5
  %26 = fadd fast float %22, %6
  %27 = fadd fast float %23, %7
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %24)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %25)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %26)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %27)  ; StoreOutput(outputSigId,rowIndex,colIndex,value)
  ret void
}

; Function Attrs: nounwind readnone
declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32) #0

; Function Attrs: nounwind
declare void @dx.op.storeOutput.f32(i32, i32, i32, i8, float) #1

; Function Attrs: nounwind readnone
declare float @dx.op.dot3.f32(i32, float, float, float, float, float, float) #0

; Function Attrs: nounwind readnone
declare float @dx.op.unary.f32(i32, float) #0

; Function Attrs: nounwind readnone
declare float @dx.op.binary.f32(i32, float, float) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.viewIdState = !{!4}
!dx.entryPoints = !{!5}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 6}
!3 = !{!"ps", i32 6, i32 0}
!4 = !{[22 x i32] [i32 20, i32 4, i32 1, i32 2, i32 4, i32 8, i32 1, i32 2, i32 4, i32 8, i32 15, i32 15, i32 15, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0]}
!5 = !{void ()* @main, !"main", !6, null, null}
!6 = !{!7, !16, null}
!7 = !{!8, !11, !12, !14, !15}
!8 = !{i32 0, !"Albedo", i8 9, i8 0, !9, i8 2, i32 1, i8 4, i32 0, i8 0, !10}
!9 = !{i32 0}
!10 = !{i32 3, i32 15}
!11 = !{i32 1, !"Emission", i8 9, i8 0, !9, i8 2, i32 1, i8 4, i32 1, i8 0, !10}
!12 = !{i32 2, !"Normal", i8 9, i8 0, !9, i8 2, i32 1, i8 3, i32 2, i8 0, !13}
!13 = !{i32 3, i32 7}
!14 = !{i32 3, !"WorldPosition", i8 9, i8 0, !9, i8 2, i32 1, i8 3, i32 3, i8 0, null}
!15 = !{i32 4, !"SV_Position", i8 9, i8 3, !9, i8 4, i32 1, i8 4, i32 4, i8 0, null}
!16 = !{!17}
!17 = !{i32 0, !"SV_Target", i8 9, i8 16, !9, i8 0, i32 1, i8 4, i32 0, i8 0, !10}

#endif

const unsigned char DrawingPS[] = {
  0x44, 0x58, 0x42, 0x43, 0x73, 0x18, 0x05, 0xf3, 0xae, 0x94, 0xa4, 0xad,
  0xb3, 0xaf, 0xe6, 0x4b, 0xe2, 0x99, 0xc8, 0xc5, 0x01, 0x00, 0x00, 0x00,
  0x93, 0x0f, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x4c, 0x00, 0x00, 0x00, 0x2d, 0x01, 0x00, 0x00, 0x67, 0x01, 0x00, 0x00,
  0x8f, 0x02, 0x00, 0x00, 0xd3, 0x08, 0x00, 0x00, 0xef, 0x08, 0x00, 0x00,
  0x53, 0x46, 0x49, 0x30, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x49, 0x53, 0x47, 0x31, 0xd9, 0x00, 0x00, 0x00,
  0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaf, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcd, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x41, 0x6c, 0x62, 0x65, 0x64, 0x6f, 0x00, 0x45, 0x6d, 0x69, 0x73, 0x73,
  0x69, 0x6f, 0x6e, 0x00, 0x4e, 0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x00, 0x57,
  0x6f, 0x72, 0x6c, 0x64, 0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e,
  0x00, 0x53, 0x56, 0x5f, 0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e,
  0x00, 0x4f, 0x53, 0x47, 0x31, 0x32, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x53, 0x56, 0x5f, 0x54, 0x61, 0x72, 0x67, 0x65, 0x74, 0x00, 0x50,
  0x53, 0x56, 0x30, 0x20, 0x01, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00,
  0x00, 0x00, 0x00, 0x05, 0x01, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x41, 0x6c, 0x62, 0x65,
  0x64, 0x6f, 0x00, 0x45, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x00,
  0x4e, 0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x00, 0x57, 0x6f, 0x72, 0x6c, 0x64,
  0x50, 0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x44, 0x00, 0x03,
  0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x44, 0x00, 0x03, 0x02, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x02, 0x43, 0x00, 0x03, 0x02, 0x00, 0x00, 0x18,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x43, 0x00, 0x03,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x04, 0x44, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x44, 0x10, 0x03, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x08,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04,
  0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f,
  0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x54, 0x41, 0x54, 0x3c,
  0x06, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x8f, 0x01, 0x00, 0x00, 0x44,
  0x58, 0x49, 0x4c, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x24,
  0x06, 0x00, 0x00, 0x42, 0x43, 0xc0, 0xde, 0x21, 0x0c, 0x00, 0x00, 0x86,
  0x01, 0x00, 0x00, 0x0b, 0x82, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13,
  0x00, 0x00, 0x00, 0x07, 0x81, 0x23, 0x91, 0x41, 0xc8, 0x04, 0x49, 0x06,
  0x10, 0x32, 0x39, 0x92, 0x01, 0x84, 0x0c, 0x25, 0x05, 0x08, 0x19, 0x1e,
  0x04, 0x8b, 0x62, 0x80, 0x14, 0x45, 0x02, 0x42, 0x92, 0x0b, 0x42, 0xa4,
  0x10, 0x32, 0x14, 0x38, 0x08, 0x18, 0x4b, 0x0a, 0x32, 0x52, 0x88, 0x48,
  0x90, 0x14, 0x20, 0x43, 0x46, 0x88, 0xa5, 0x00, 0x19, 0x32, 0x42, 0xe4,
  0x48, 0x0e, 0x90, 0x91, 0x22, 0xc4, 0x50, 0x41, 0x51, 0x81, 0x8c, 0xe1,
  0x83, 0xe5, 0x8a, 0x04, 0x29, 0x46, 0x06, 0x51, 0x18, 0x00, 0x00, 0x06,
  0x00, 0x00, 0x00, 0x1b, 0x8c, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x40,
  0x02, 0xa8, 0x0d, 0x84, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x03, 0x20, 0x01,
  0x00, 0x00, 0x00, 0x49, 0x18, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13,
  0x82, 0x60, 0x42, 0x20, 0x00, 0x00, 0x00, 0x89, 0x20, 0x00, 0x00, 0x1a,
  0x00, 0x00, 0x00, 0x32, 0x22, 0x48, 0x09, 0x20, 0x64, 0x85, 0x04, 0x93,
  0x22, 0xa4, 0x84, 0x04, 0x93, 0x22, 0xe3, 0x84, 0xa1, 0x90, 0x14, 0x12,
  0x4c, 0x8a, 0x8c, 0x0b, 0x84, 0xa4, 0x4c, 0x10, 0x54, 0x23, 0x00, 0x25,
  0x00, 0x14, 0x66, 0x00, 0xe6, 0x08, 0xc0, 0x60, 0x8e, 0x00, 0x29, 0xc6,
  0x20, 0x84, 0x14, 0x42, 0xa6, 0x18, 0x80, 0x10, 0x52, 0x06, 0xa1, 0x82,
  0x0c, 0x32, 0xc6, 0x18, 0x63, 0x90, 0x2a, 0xc3, 0x20, 0x83, 0x58, 0x21,
  0x06, 0x19, 0x83, 0xdc, 0x40, 0xc0, 0x30, 0x02, 0x31, 0x0c, 0x23, 0x0c,
  0xc3, 0x39, 0xd2, 0x14, 0x51, 0xc2, 0xe4, 0xa7, 0xc8, 0x45, 0x2c, 0x62,
  0x83, 0x8a, 0x18, 0xa5, 0x8c, 0x69, 0x41, 0x00, 0x00, 0x00, 0x00, 0x13,
  0x14, 0x72, 0xc0, 0x87, 0x74, 0x60, 0x87, 0x36, 0x68, 0x87, 0x79, 0x68,
  0x03, 0x72, 0xc0, 0x87, 0x0d, 0xaf, 0x50, 0x0e, 0x6d, 0xd0, 0x0e, 0x7a,
  0x50, 0x0e, 0x6d, 0x00, 0x0f, 0x7a, 0x30, 0x07, 0x72, 0xa0, 0x07, 0x73,
  0x20, 0x07, 0x6d, 0x90, 0x0e, 0x71, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d,
  0x90, 0x0e, 0x78, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x71,
  0x60, 0x07, 0x7a, 0x30, 0x07, 0x72, 0xd0, 0x06, 0xe9, 0x30, 0x07, 0x72,
  0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x76, 0x40, 0x07, 0x7a,
  0x60, 0x07, 0x74, 0xd0, 0x06, 0xe6, 0x10, 0x07, 0x76, 0xa0, 0x07, 0x73,
  0x20, 0x07, 0x6d, 0x60, 0x0e, 0x73, 0x20, 0x07, 0x7a, 0x30, 0x07, 0x72,
  0xd0, 0x06, 0xe6, 0x60, 0x07, 0x74, 0xa0, 0x07, 0x76, 0x40, 0x07, 0x6d,
  0xe0, 0x0e, 0x78, 0xa0, 0x07, 0x71, 0x60, 0x07, 0x7a, 0x30, 0x07, 0x72,
  0xa0, 0x07, 0x76, 0x40, 0x07, 0x43, 0x9e, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x3c, 0x06, 0x10, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x79, 0x10, 0x20, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xf2, 0x28, 0x40,
  0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xe4, 0x61,
  0x80, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xc8,
  0xe3, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
  0x16, 0x08, 0x00, 0x13, 0x00, 0x00, 0x00, 0x32, 0x1e, 0x98, 0x14, 0x19,
  0x11, 0x4c, 0x90, 0x8c, 0x09, 0x26, 0x47, 0xc6, 0x04, 0x43, 0x22, 0x25,
  0x30, 0x02, 0x50, 0x0c, 0x05, 0x1a, 0x50, 0x06, 0x85, 0x50, 0x0e, 0x25,
  0x51, 0x20, 0x05, 0x54, 0x60, 0x05, 0x18, 0x50, 0x1e, 0x45, 0x50, 0x16,
  0x54, 0x4a, 0x62, 0x04, 0xa0, 0x08, 0x0a, 0xa1, 0x0c, 0x0a, 0x84, 0xe6,
  0x0c, 0x00, 0xd1, 0xb1, 0x2c, 0x8a, 0x08, 0x04, 0x02, 0x09, 0x04, 0x02,
  0x79, 0x9e, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79,
  0x18, 0x00, 0x00, 0x95, 0x00, 0x00, 0x00, 0x1a, 0x03, 0x4c, 0x90, 0x46,
  0x02, 0x13, 0x44, 0x35, 0x18, 0x63, 0x0b, 0x73, 0x3b, 0x03, 0xb1, 0x2b,
  0x93, 0x9b, 0x4b, 0x7b, 0x73, 0x03, 0x99, 0x71, 0xb9, 0x01, 0x41, 0xa1,
  0x0b, 0x3b, 0x9b, 0x7b, 0x91, 0x2a, 0x62, 0x2a, 0x0a, 0x9a, 0x2a, 0xfa,
  0x9a, 0xb9, 0x81, 0x79, 0x31, 0x4b, 0x73, 0x0b, 0x63, 0x4b, 0xd9, 0x10,
  0x04, 0x13, 0x04, 0xc2, 0x98, 0x20, 0x10, 0xc7, 0x06, 0x61, 0x20, 0x36,
  0x08, 0x04, 0x41, 0x01, 0x6e, 0x6e, 0x82, 0x40, 0x20, 0x1b, 0x86, 0x03,
  0x21, 0x26, 0x08, 0xd3, 0x36, 0x41, 0x20, 0x12, 0x1a, 0x41, 0x6c, 0x62,
  0x65, 0x64, 0x6f, 0x13, 0x04, 0x42, 0x99, 0x20, 0x10, 0xcb, 0x04, 0x81,
  0x60, 0x26, 0x08, 0x44, 0xb3, 0x01, 0x41, 0x98, 0x86, 0x70, 0x98, 0x07,
  0x22, 0x52, 0xd4, 0x96, 0x36, 0x37, 0x97, 0xf6, 0xe6, 0x36, 0x41, 0x20,
  0x9c, 0x0d, 0x08, 0x22, 0x35, 0x93, 0x23, 0x3d, 0x10, 0x0d, 0xa7, 0x37,
  0xb9, 0xb6, 0x30, 0xb6, 0x09, 0x02, 0xf1, 0x6c, 0x40, 0x90, 0xaa, 0xb1,
  0x9c, 0xea, 0x81, 0xd8, 0x5c, 0xbd, 0xc9, 0xb1, 0x91, 0x41, 0xbd, 0xcd,
  0xa5, 0xd1, 0xa5, 0xbd, 0xb9, 0x4d, 0x10, 0x08, 0x68, 0x03, 0x82, 0x60,
  0x4d, 0xe6, 0x60, 0x0f, 0x44, 0x04, 0xea, 0x6d, 0x2e, 0x8d, 0x2e, 0xed,
  0xcd, 0x6d, 0x82, 0x40, 0x44, 0x5c, 0xa6, 0xac, 0xbe, 0xa0, 0xde, 0xe6,
  0xd2, 0xe8, 0xd2, 0xde, 0xdc, 0x36, 0x20, 0xc8, 0xd6, 0x70, 0x4e, 0xf7,
  0x40, 0x1b, 0x8c, 0x25, 0xa2, 0x2e, 0xcd, 0xdb, 0x30, 0x10, 0xca, 0x37,
  0x41, 0x10, 0x80, 0x0d, 0xc0, 0x86, 0x81, 0x10, 0x03, 0x31, 0xd8, 0x10,
  0x8c, 0xc1, 0x86, 0x61, 0x08, 0x03, 0x32, 0x98, 0x20, 0x50, 0xdc, 0x86,
  0xc0, 0x0c, 0x48, 0xb4, 0x85, 0xa5, 0xb9, 0x4d, 0x10, 0x8a, 0x6a, 0x82,
  0x50, 0x58, 0x1b, 0x02, 0x62, 0x82, 0x50, 0x5c, 0x13, 0x84, 0x02, 0x9b,
  0x20, 0x10, 0xd2, 0x06, 0xa1, 0x71, 0x83, 0x0d, 0x0b, 0xc1, 0xa4, 0x81,
  0x1a, 0xac, 0x01, 0x1b, 0x0c, 0x6d, 0x40, 0xa8, 0xc1, 0x1b, 0x6c, 0x58,
  0x06, 0x29, 0x0d, 0xd4, 0x60, 0x0d, 0xd8, 0x60, 0x68, 0x83, 0x41, 0x0d,
  0xde, 0x60, 0x82, 0x40, 0x4c, 0x13, 0x84, 0x22, 0xdb, 0x20, 0x34, 0xcf,
  0x86, 0x45, 0x0e, 0xaa, 0x34, 0x50, 0x83, 0x35, 0x60, 0x83, 0x61, 0x0e,
  0xe4, 0x40, 0x0d, 0xe8, 0x60, 0xc3, 0xd2, 0x60, 0x69, 0xa0, 0x06, 0x6b,
  0xc0, 0x06, 0xc3, 0x1c, 0x34, 0x6a, 0x00, 0x6c, 0x58, 0x9c, 0x2e, 0x0d,
  0xe6, 0x60, 0x0d, 0xda, 0x60, 0x68, 0x03, 0x47, 0x0d, 0x80, 0x0d, 0x05,
  0x1c, 0xc4, 0x41, 0x1d, 0xd8, 0xc1, 0x1d, 0x30, 0x99, 0xb2, 0xfa, 0xa2,
  0x0a, 0x93, 0x3b, 0x2b, 0xa3, 0x9b, 0x20, 0x14, 0xda, 0x86, 0x85, 0xc8,
  0x83, 0x34, 0xd0, 0x83, 0x35, 0x50, 0x83, 0xa1, 0x0d, 0x08, 0x35, 0x78,
  0x83, 0x0d, 0xc1, 0x1e, 0x6c, 0x18, 0xf0, 0x80, 0x0f, 0x80, 0x0d, 0x45,
  0x18, 0xa0, 0x41, 0x1f, 0x00, 0x00, 0x0d, 0x33, 0xb6, 0xb7, 0x30, 0xba,
  0x39, 0x16, 0x69, 0x6e, 0x73, 0x74, 0x73, 0x34, 0xe6, 0xd2, 0xce, 0xbe,
  0xd8, 0xc8, 0x26, 0x08, 0x04, 0x45, 0x63, 0x2e, 0xed, 0xec, 0x6b, 0x8e,
  0x6e, 0x03, 0xf2, 0x07, 0x13, 0x28, 0x58, 0xa1, 0x20, 0x0a, 0xa3, 0xe0,
  0x54, 0x61, 0x63, 0xb3, 0x6b, 0x73, 0x49, 0x23, 0x2b, 0x73, 0xa3, 0x9b,
  0x12, 0x04, 0x55, 0xc8, 0xf0, 0x5c, 0xec, 0xca, 0xe4, 0xe6, 0xd2, 0xde,
  0xdc, 0xa6, 0x04, 0x44, 0x13, 0x32, 0x3c, 0x17, 0xbb, 0x30, 0x36, 0xbb,
  0x32, 0xb9, 0x29, 0x41, 0x51, 0x87, 0x0c, 0xcf, 0x65, 0x0e, 0x2d, 0x8c,
  0xac, 0x4c, 0xae, 0xe9, 0x8d, 0xac, 0x8c, 0x6d, 0x4a, 0x80, 0x54, 0x22,
  0xc3, 0x73, 0xa1, 0xcb, 0x83, 0x2b, 0x0b, 0x72, 0x73, 0x7b, 0xa3, 0x0b,
  0xa3, 0x4b, 0x7b, 0x73, 0x9b, 0x9b, 0x22, 0x7c, 0x64, 0x50, 0x87, 0x0c,
  0xcf, 0xc5, 0x2e, 0xad, 0xec, 0x2e, 0x89, 0x6c, 0x8a, 0x2e, 0x8c, 0xae,
  0x6c, 0x4a, 0x60, 0x06, 0x75, 0xc8, 0xf0, 0x5c, 0xca, 0xdc, 0xe8, 0xe4,
  0xf2, 0xa0, 0xde, 0xd2, 0xdc, 0xe8, 0xe6, 0xa6, 0x04, 0x7d, 0xd0, 0x85,
  0x0c, 0xcf, 0x65, 0xec, 0xad, 0xce, 0x8d, 0xae, 0x4c, 0x6e, 0x6e, 0x4a,
  0x30, 0x0a, 0x00, 0x79, 0x18, 0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x33,
  0x08, 0x80, 0x1c, 0xc4, 0xe1, 0x1c, 0x66, 0x14, 0x01, 0x3d, 0x88, 0x43,
  0x38, 0x84, 0xc3, 0x8c, 0x42, 0x80, 0x07, 0x79, 0x78, 0x07, 0x73, 0x98,
  0x71, 0x0c, 0xe6, 0x00, 0x0f, 0xed, 0x10, 0x0e, 0xf4, 0x80, 0x0e, 0x33,
  0x0c, 0x42, 0x1e, 0xc2, 0xc1, 0x1d, 0xce, 0xa1, 0x1c, 0x66, 0x30, 0x05,
  0x3d, 0x88, 0x43, 0x38, 0x84, 0x83, 0x1b, 0xcc, 0x03, 0x3d, 0xc8, 0x43,
  0x3d, 0x8c, 0x03, 0x3d, 0xcc, 0x78, 0x8c, 0x74, 0x70, 0x07, 0x7b, 0x08,
  0x07, 0x79, 0x48, 0x87, 0x70, 0x70, 0x07, 0x7a, 0x70, 0x03, 0x76, 0x78,
  0x87, 0x70, 0x20, 0x87, 0x19, 0xcc, 0x11, 0x0e, 0xec, 0x90, 0x0e, 0xe1,
  0x30, 0x0f, 0x6e, 0x30, 0x0f, 0xe3, 0xf0, 0x0e, 0xf0, 0x50, 0x0e, 0x33,
  0x10, 0xc4, 0x1d, 0xde, 0x21, 0x1c, 0xd8, 0x21, 0x1d, 0xc2, 0x61, 0x1e,
  0x66, 0x30, 0x89, 0x3b, 0xbc, 0x83, 0x3b, 0xd0, 0x43, 0x39, 0xb4, 0x03,
  0x3c, 0xbc, 0x83, 0x3c, 0x84, 0x03, 0x3b, 0xcc, 0xf0, 0x14, 0x76, 0x60,
  0x07, 0x7b, 0x68, 0x07, 0x37, 0x68, 0x87, 0x72, 0x68, 0x07, 0x37, 0x80,
  0x87, 0x70, 0x90, 0x87, 0x70, 0x60, 0x07, 0x76, 0x28, 0x07, 0x76, 0xf8,
  0x05, 0x76, 0x78, 0x87, 0x77, 0x80, 0x87, 0x5f, 0x08, 0x87, 0x71, 0x18,
  0x87, 0x72, 0x98, 0x87, 0x79, 0x98, 0x81, 0x2c, 0xee, 0xf0, 0x0e, 0xee,
  0xe0, 0x0e, 0xf5, 0xc0, 0x0e, 0xec, 0x30, 0x03, 0x62, 0xc8, 0xa1, 0x1c,
  0xe4, 0xa1, 0x1c, 0xcc, 0xa1, 0x1c, 0xe4, 0xa1, 0x1c, 0xdc, 0x61, 0x1c,
  0xca, 0x21, 0x1c, 0xc4, 0x81, 0x1d, 0xca, 0x61, 0x06, 0xd6, 0x90, 0x43,
  0x39, 0xc8, 0x43, 0x39, 0x98, 0x43, 0x39, 0xc8, 0x43, 0x39, 0xb8, 0xc3,
  0x38, 0x94, 0x43, 0x38, 0x88, 0x03, 0x3b, 0x94, 0xc3, 0x2f, 0xbc, 0x83,
  0x3c, 0xfc, 0x82, 0x3b, 0xd4, 0x03, 0x3b, 0xb0, 0xc3, 0x0c, 0xc4, 0x21,
  0x07, 0x7c, 0x70, 0x03, 0x7a, 0x28, 0x87, 0x76, 0x80, 0x87, 0x19, 0xd1,
  0x43, 0x0e, 0xf8, 0xe0, 0x06, 0xe4, 0x20, 0x0e, 0xe7, 0xe0, 0x06, 0xf6,
  0x10, 0x0e, 0xf2, 0xc0, 0x0e, 0xe1, 0x90, 0x0f, 0xef, 0x50, 0x0f, 0xf4,
  0x00, 0x00, 0x00, 0x71, 0x20, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x56,
  0x00, 0x0d, 0x97, 0xef, 0x3c, 0x7e, 0x80, 0x34, 0x40, 0x84, 0xf9, 0xc5,
  0x6d, 0xdb, 0x80, 0x33, 0x5c, 0xbe, 0xf3, 0xf8, 0x83, 0x33, 0xdd, 0x7e,
  0x71, 0xdb, 0x16, 0x30, 0x0d, 0x97, 0xef, 0x3c, 0xfe, 0xe2, 0x00, 0x83,
  0xd8, 0x3c, 0xd4, 0xe4, 0x17, 0xb7, 0x6d, 0x02, 0xd5, 0x70, 0xf9, 0xce,
  0xe3, 0x4b, 0x93, 0x13, 0x11, 0x28, 0x35, 0x3d, 0xd4, 0xe4, 0x17, 0xb7,
  0x6d, 0x04, 0xcf, 0x70, 0xf9, 0xce, 0xe3, 0x53, 0x0d, 0x10, 0x61, 0x7e,
  0x71, 0xdb, 0x06, 0x40, 0x30, 0x00, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x48, 0x41, 0x53, 0x48, 0x14, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x19, 0x44, 0x00, 0xa8, 0x31, 0x48, 0xea, 0x2b, 0x1b,
  0x3a, 0x91, 0x76, 0x92, 0x25, 0x8a, 0xd1, 0x44, 0x58, 0x49, 0x4c, 0x9c,
  0x06, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0xa7, 0x01, 0x00, 0x00, 0x44,
  0x58, 0x49, 0x4c, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x84,
  0x06, 0x00, 0x00, 0x42, 0x43, 0xc0, 0xde, 0x21, 0x0c, 0x00, 0x00, 0x9e,
  0x01, 0x00, 0x00, 0x0b, 0x82, 0x20, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13,
  0x00, 0x00, 0x00, 0x07, 0x81, 0x23, 0x91, 0x41, 0xc8, 0x04, 0x49, 0x06,
  0x10, 0x32, 0x39, 0x92, 0x01, 0x84, 0x0c, 0x25, 0x05, 0x08, 0x19, 0x1e,
  0x04, 0x8b, 0x62, 0x80, 0x14, 0x45, 0x02, 0x42, 0x92, 0x0b, 0x42, 0xa4,
  0x10, 0x32, 0x14, 0x38, 0x08, 0x18, 0x4b, 0x0a, 0x32, 0x52, 0x88, 0x48,
  0x90, 0x14, 0x20, 0x43, 0x46, 0x88, 0xa5, 0x00, 0x19, 0x32, 0x42, 0xe4,
  0x48, 0x0e, 0x90, 0x91, 0x22, 0xc4, 0x50, 0x41, 0x51, 0x81, 0x8c, 0xe1,
  0x83, 0xe5, 0x8a, 0x04, 0x29, 0x46, 0x06, 0x51, 0x18, 0x00, 0x00, 0x06,
  0x00, 0x00, 0x00, 0x1b, 0x8c, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x07, 0x40,
  0x02, 0xa8, 0x0d, 0x84, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x03, 0x20, 0x01,
  0x00, 0x00, 0x00, 0x49, 0x18, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13,
  0x82, 0x60, 0x42, 0x20, 0x00, 0x00, 0x00, 0x89, 0x20, 0x00, 0x00, 0x14,
  0x00, 0x00, 0x00, 0x32, 0x22, 0x48, 0x09, 0x20, 0x64, 0x85, 0x04, 0x93,
  0x22, 0xa4, 0x84, 0x04, 0x93, 0x22, 0xe3, 0x84, 0xa1, 0x90, 0x14, 0x12,
  0x4c, 0x8a, 0x8c, 0x0b, 0x84, 0xa4, 0x4c, 0x10, 0x48, 0x23, 0x00, 0x25,
  0x00, 0x14, 0x66, 0x00, 0xe6, 0x08, 0xc0, 0x60, 0x8e, 0x00, 0x29, 0xc6,
  0x20, 0x84, 0x14, 0x42, 0xa6, 0x18, 0x80, 0x10, 0x52, 0x06, 0xa1, 0x82,
  0x0c, 0x32, 0xc6, 0x18, 0x63, 0x90, 0x2a, 0xc3, 0x20, 0x83, 0x58, 0x21,
  0x06, 0x19, 0x83, 0xdc, 0x40, 0x40, 0x5a, 0x10, 0x00, 0x00, 0x00, 0x13,
  0x14, 0x72, 0xc0, 0x87, 0x74, 0x60, 0x87, 0x36, 0x68, 0x87, 0x79, 0x68,
  0x03, 0x72, 0xc0, 0x87, 0x0d, 0xaf, 0x50, 0x0e, 0x6d, 0xd0, 0x0e, 0x7a,
  0x50, 0x0e, 0x6d, 0x00, 0x0f, 0x7a, 0x30, 0x07, 0x72, 0xa0, 0x07, 0x73,
  0x20, 0x07, 0x6d, 0x90, 0x0e, 0x71, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d,
  0x90, 0x0e, 0x78, 0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x71,
  0x60, 0x07, 0x7a, 0x30, 0x07, 0x72, 0xd0, 0x06, 0xe9, 0x30, 0x07, 0x72,
  0xa0, 0x07, 0x73, 0x20, 0x07, 0x6d, 0x90, 0x0e, 0x76, 0x40, 0x07, 0x7a,
  0x60, 0x07, 0x74, 0xd0, 0x06, 0xe6, 0x10, 0x07, 0x76, 0xa0, 0x07, 0x73,
  0x20, 0x07, 0x6d, 0x60, 0x0e, 0x73, 0x20, 0x07, 0x7a, 0x30, 0x07, 0x72,
  0xd0, 0x06, 0xe6, 0x60, 0x07, 0x74, 0xa0, 0x07, 0x76, 0x40, 0x07, 0x6d,
  0xe0, 0x0e, 0x78, 0xa0, 0x07, 0x71, 0x60, 0x07, 0x7a, 0x30, 0x07, 0x72,
  0xa0, 0x07, 0x76, 0x40, 0x07, 0x43, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x3c, 0x06, 0x10, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x79, 0x10, 0x20, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xf2, 0x28, 0x40,
  0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xe4, 0x61,
  0x80, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0xc8,
  0xe3, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
  0x16, 0x08, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x32, 0x1e, 0x98, 0x14, 0x19,
  0x11, 0x4c, 0x90, 0x8c, 0x09, 0x26, 0x47, 0xc6, 0x04, 0x43, 0x22, 0x25,
  0x30, 0x02, 0x50, 0x0c, 0x65, 0x50, 0x1e, 0x45, 0x50, 0x0e, 0x85, 0x40,
  0xa5, 0x24, 0x46, 0x00, 0x8a, 0xa0, 0x10, 0xca, 0xa0, 0x40, 0x28, 0x8e,
  0x65, 0x51, 0x44, 0x20, 0x10, 0x48, 0x20, 0x10, 0xc8, 0xf3, 0x3c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79, 0x18, 0x00, 0x00, 0x60,
  0x00, 0x00, 0x00, 0x1a, 0x03, 0x4c, 0x90, 0x46, 0x02, 0x13, 0x44, 0x35,
  0x18, 0x63, 0x0b, 0x73, 0x3b, 0x03, 0xb1, 0x2b, 0x93, 0x9b, 0x4b, 0x7b,
  0x73, 0x03, 0x99, 0x71, 0xb9, 0x01, 0x41, 0xa1, 0x0b, 0x3b, 0x9b, 0x7b,
  0x91, 0x2a, 0x62, 0x2a, 0x0a, 0x9a, 0x2a, 0xfa, 0x9a, 0xb9, 0x81, 0x79,
  0x31, 0x4b, 0x73, 0x0b, 0x63, 0x4b, 0xd9, 0x10, 0x04, 0x13, 0x04, 0xc2,
  0x98, 0x20, 0x10, 0xc7, 0x06, 0x61, 0x20, 0x26, 0x08, 0x04, 0xb2, 0x41,
  0x18, 0x0c, 0x0a, 0x70, 0x73, 0x1b, 0x06, 0xc4, 0x20, 0x26, 0x08, 0x11,
  0xb5, 0x21, 0x50, 0x26, 0x08, 0x02, 0x40, 0xa2, 0x2d, 0x2c, 0xcd, 0x8d,
  0x46, 0x10, 0x9b, 0x58, 0x19, 0xd9, 0xdb, 0x04, 0xa1, 0x70, 0x26, 0x08,
  0xc5, 0xb3, 0x21, 0x20, 0x26, 0x08, 0x05, 0x34, 0x41, 0x28, 0xa2, 0x09,
  0x02, 0x91, 0x4c, 0x10, 0x08, 0x65, 0x83, 0x40, 0x55, 0x1b, 0x16, 0xc2,
  0x79, 0xa0, 0x48, 0x1a, 0x26, 0x02, 0xb2, 0x88, 0x14, 0xb5, 0xa5, 0xcd,
  0xcd, 0xa5, 0xbd, 0xb9, 0x6d, 0x58, 0x06, 0xec, 0x81, 0x22, 0x69, 0x98,
  0x06, 0xc8, 0x9a, 0x20, 0x10, 0x0b, 0x0d, 0xa7, 0x37, 0xb9, 0xb6, 0x30,
  0xb6, 0x09, 0x42, 0x21, 0x4d, 0x10, 0x08, 0x66, 0x83, 0x40, 0x75, 0x1b,
  0x16, 0x6d, 0x7b, 0xa0, 0x48, 0x1a, 0x38, 0x0d, 0xf2, 0xd8, 0x5c, 0xbd,
  0xc9, 0xb1, 0x91, 0x41, 0xbd, 0xcd, 0xa5, 0xd1, 0xa5, 0xbd, 0xb9, 0x6d,
  0x58, 0x28, 0x30, 0x78, 0xa0, 0x48, 0x1a, 0x38, 0x0a, 0x02, 0x26, 0x08,
  0x44, 0xc3, 0x65, 0xca, 0xea, 0x0b, 0xea, 0x6d, 0x2e, 0x8d, 0x2e, 0xed,
  0xcd, 0x6d, 0xc3, 0x22, 0x06, 0x63, 0xf0, 0x70, 0xd1, 0x34, 0x4c, 0x62,
  0x00, 0x01, 0x1b, 0x8a, 0x2b, 0xfb, 0xc2, 0x80, 0x0c, 0x98, 0x4c, 0x59,
  0x7d, 0x51, 0x85, 0xc9, 0x9d, 0x95, 0xd1, 0x4d, 0x10, 0x8a, 0x69, 0xc3,
  0x42, 0x98, 0xc1, 0x73, 0x06, 0x11, 0x34, 0x4c, 0x04, 0x64, 0x6d, 0x08,
  0xd0, 0x60, 0xc3, 0x50, 0x06, 0x69, 0x00, 0x6c, 0x28, 0x98, 0x46, 0x0d,
  0x00, 0xa0, 0x0a, 0x1b, 0x9b, 0x5d, 0x9b, 0x4b, 0x1a, 0x59, 0x99, 0x1b,
  0xdd, 0x94, 0x20, 0xa8, 0x42, 0x86, 0xe7, 0x62, 0x57, 0x26, 0x37, 0x97,
  0xf6, 0xe6, 0x36, 0x25, 0x20, 0x9a, 0x90, 0xe1, 0xb9, 0xd8, 0x85, 0xb1,
  0xd9, 0x95, 0xc9, 0x4d, 0x09, 0x8c, 0x3a, 0x64, 0x78, 0x2e, 0x73, 0x68,
  0x61, 0x64, 0x65, 0x72, 0x4d, 0x6f, 0x64, 0x65, 0x6c, 0x53, 0x02, 0xa4,
  0x0e, 0x19, 0x9e, 0x8b, 0x5d, 0x5a, 0xd9, 0x5d, 0x12, 0xd9, 0x14, 0x5d,
  0x18, 0x5d, 0xd9, 0x94, 0x40, 0xa9, 0x43, 0x86, 0xe7, 0x52, 0xe6, 0x46,
  0x27, 0x97, 0x07, 0xf5, 0x96, 0xe6, 0x46, 0x37, 0x37, 0x25, 0x50, 0x03,
  0x00, 0x00, 0x00, 0x79, 0x18, 0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x33,
  0x08, 0x80, 0x1c, 0xc4, 0xe1, 0x1c, 0x66, 0x14, 0x01, 0x3d, 0x88, 0x43,
  0x38, 0x84, 0xc3, 0x8c, 0x42, 0x80, 0x07, 0x79, 0x78, 0x07, 0x73, 0x98,
  0x71, 0x0c, 0xe6, 0x00, 0x0f, 0xed, 0x10, 0x0e, 0xf4, 0x80, 0x0e, 0x33,
  0x0c, 0x42, 0x1e, 0xc2, 0xc1, 0x1d, 0xce, 0xa1, 0x1c, 0x66, 0x30, 0x05,
  0x3d, 0x88, 0x43, 0x38, 0x84, 0x83, 0x1b, 0xcc, 0x03, 0x3d, 0xc8, 0x43,
  0x3d, 0x8c, 0x03, 0x3d, 0xcc, 0x78, 0x8c, 0x74, 0x70, 0x07, 0x7b, 0x08,
  0x07, 0x79, 0x48, 0x87, 0x70, 0x70, 0x07, 0x7a, 0x70, 0x03, 0x76, 0x78,
  0x87, 0x70, 0x20, 0x87, 0x19, 0xcc, 0x11, 0x0e, 0xec, 0x90, 0x0e, 0xe1,
  0x30, 0x0f, 0x6e, 0x30, 0x0f, 0xe3, 0xf0, 0x0e, 0xf0, 0x50, 0x0e, 0x33,
  0x10, 0xc4, 0x1d, 0xde, 0x21, 0x1c, 0xd8, 0x21, 0x1d, 0xc2, 0x61, 0x1e,
  0x66, 0x30, 0x89, 0x3b, 0xbc, 0x83, 0x3b, 0xd0, 0x43, 0x39, 0xb4, 0x03,
  0x3c, 0xbc, 0x83, 0x3c, 0x84, 0x03, 0x3b, 0xcc, 0xf0, 0x14, 0x76, 0x60,
  0x07, 0x7b, 0x68, 0x07, 0x37, 0x68, 0x87, 0x72, 0x68, 0x07, 0x37, 0x80,
  0x87, 0x70, 0x90, 0x87, 0x70, 0x60, 0x07, 0x76, 0x28, 0x07, 0x76, 0xf8,
  0x05, 0x76, 0x78, 0x87, 0x77, 0x80, 0x87, 0x5f, 0x08, 0x87, 0x71, 0x18,
  0x87, 0x72, 0x98, 0x87, 0x79, 0x98, 0x81, 0x2c, 0xee, 0xf0, 0x0e, 0xee,
  0xe0, 0x0e, 0xf5, 0xc0, 0x0e, 0xec, 0x30, 0x03, 0x62, 0xc8, 0xa1, 0x1c,
  0xe4, 0xa1, 0x1c, 0xcc, 0xa1, 0x1c, 0xe4, 0xa1, 0x1c, 0xdc, 0x61, 0x1c,
  0xca, 0x21, 0x1c, 0xc4, 0x81, 0x1d, 0xca, 0x61, 0x06, 0xd6, 0x90, 0x43,
  0x39, 0xc8, 0x43, 0x39, 0x98, 0x43, 0x39, 0xc8, 0x43, 0x39, 0xb8, 0xc3,
  0x38, 0x94, 0x43, 0x38, 0x88, 0x03, 0x3b, 0x94, 0xc3, 0x2f, 0xbc, 0x83,
  0x3c, 0xfc, 0x82, 0x3b, 0xd4, 0x03, 0x3b, 0xb0, 0xc3, 0x0c, 0xc4, 0x21,
  0x07, 0x7c, 0x70, 0x03, 0x7a, 0x28, 0x87, 0x76, 0x80, 0x87, 0x19, 0xd1,
  0x43, 0x0e, 0xf8, 0xe0, 0x06, 0xe4, 0x20, 0x0e, 0xe7, 0xe0, 0x06, 0xf6,
  0x10, 0x0e, 0xf2, 0xc0, 0x0e, 0xe1, 0x90, 0x0f, 0xef, 0x50, 0x0f, 0xf4,
  0x00, 0x00, 0x00, 0x71, 0x20, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x56,
  0x00, 0x0d, 0x97, 0xef, 0x3c, 0x7e, 0x80, 0x34, 0x40, 0x84, 0xf9, 0xc5,
  0x6d, 0xdb, 0x80, 0x33, 0x5c, 0xbe, 0xf3, 0xf8, 0x83, 0x33, 0xdd, 0x7e,
  0x71, 0xdb, 0x16, 0x30, 0x0d, 0x97, 0xef, 0x3c, 0xfe, 0xe2, 0x00, 0x83,
  0xd8, 0x3c, 0xd4, 0xe4, 0x17, 0xb7, 0x6d, 0x02, 0xd5, 0x70, 0xf9, 0xce,
  0xe3, 0x4b, 0x93, 0x13, 0x11, 0x28, 0x35, 0x3d, 0xd4, 0xe4, 0x17, 0xb7,
  0x6d, 0x04, 0xcf, 0x70, 0xf9, 0xce, 0xe3, 0x53, 0x0d, 0x10, 0x61, 0x7e,
  0x71, 0xdb, 0x06, 0x40, 0x30, 0x00, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x61,
  0x20, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x13, 0x04, 0x41, 0x2c, 0x10,
  0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x54, 0x4a, 0x80, 0x48, 0x19,
  0x95, 0xcc, 0x0c, 0x40, 0x29, 0x94, 0x1b, 0x8d, 0x31, 0x82, 0xd6, 0x9c,
  0x73, 0xda, 0x1b, 0x23, 0x20, 0x59, 0xbf, 0x54, 0xbf, 0x31, 0x02, 0x10,
  0x04, 0x41, 0xfc, 0x03, 0x00, 0x00, 0x00, 0x23, 0x06, 0x09, 0x00, 0x82,
  0x60, 0x60, 0x74, 0xd1, 0x74, 0x3d, 0xc6, 0x88, 0x41, 0x02, 0x80, 0x20,
  0x18, 0x18, 0x9e, 0x44, 0x61, 0xca, 0x31, 0x62, 0x90, 0x00, 0x20, 0x08,
  0x06, 0xc6, 0x37, 0x55, 0x19, 0x84, 0x8c, 0x18, 0x24, 0x00, 0x08, 0x82,
  0x81, 0x01, 0x06, 0xd4, 0xa6, 0x49, 0xc9, 0x88, 0x41, 0x02, 0x80, 0x20,
  0x18, 0x18, 0x61, 0x50, 0x71, 0x5b, 0xa3, 0x8c, 0x18, 0x24, 0x00, 0x08,
  0x82, 0x81, 0x21, 0x06, 0x56, 0xc7, 0x4d, 0xcb, 0x88, 0x41, 0x02, 0x80,
  0x20, 0x18, 0x18, 0x63, 0x70, 0x79, 0x9d, 0xc4, 0x8c, 0x18, 0x24, 0x00,
  0x08, 0x82, 0x81, 0x41, 0x06, 0x98, 0xe7, 0x59, 0xcd, 0x88, 0x41, 0x02,
  0x80, 0x20, 0x18, 0x18, 0x65, 0x90, 0x7d, 0x5f, 0xe4, 0x8c, 0x18, 0x24,
  0x00, 0x08, 0x82, 0x81, 0x61, 0x06, 0x1a, 0x18, 0x80, 0xc1, 0xf5, 0x8c,
  0x18, 0x24, 0x00, 0x08, 0x82, 0x81, 0x71, 0x06, 0x5b, 0x18, 0x84, 0x81,
  0x05, 0x8d, 0x18, 0x2c, 0x00, 0x08, 0x82, 0x81, 0x62, 0x06, 0xcf, 0xa2,
  0x24, 0x8b, 0x92, 0x8c, 0x18, 0x18, 0x00, 0x08, 0x82, 0x01, 0x63, 0x06,
  0x53, 0x60, 0x41, 0x23, 0x1f, 0x13, 0x1a, 0xf9, 0xd8, 0xd0, 0xc8, 0x67,
  0xc4, 0x60, 0x01, 0x40, 0x10, 0x0c, 0x94, 0x35, 0xa0, 0x06, 0x21, 0x88,
  0xa2, 0xc8, 0x82, 0x49, 0x3e, 0x23, 0x06, 0x07, 0x00, 0x82, 0x60, 0xe0,
  0xac, 0x81, 0x56, 0x05, 0x16, 0x30, 0xf2, 0x31, 0x81, 0x91, 0x8f, 0x0d,
  0x8c, 0x7c, 0x8c, 0x60, 0xe4, 0x63, 0x04, 0x05, 0x1f, 0x23, 0x28, 0xf8,
  0x18, 0x41, 0xc1, 0xc7, 0x08, 0x0a, 0x3e, 0x23, 0x06, 0x09, 0x00, 0x82,
  0x60, 0x80, 0xdc, 0x01, 0x18, 0xc8, 0x81, 0x1c, 0xa8, 0x01, 0x31, 0x62,
  0x90, 0x00, 0x20, 0x08, 0x06, 0xc8, 0x1d, 0x80, 0x81, 0x1c, 0xc8, 0x01,
  0x19, 0x0c, 0x23, 0x06, 0x09, 0x00, 0x82, 0x60, 0x80, 0xdc, 0x01, 0x18,
  0xc8, 0x81, 0x1c, 0xa4, 0x81, 0x30, 0x62, 0x90, 0x00, 0x20, 0x08, 0x06,
  0xc8, 0x1d, 0x80, 0x81, 0x1c, 0xc8, 0xc1, 0x19, 0x04, 0x08, 0x00, 0x00,
  0x00, 0x00, 0x00
};
