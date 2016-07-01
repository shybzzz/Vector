object MainForm: TMainForm
  Left = 354
  Top = 115
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = #1052#1086#1076#1091#1083#1100' E-310. '#1043#1077#1085#1077#1088#1072#1090#1086#1088'.'
  ClientHeight = 811
  ClientWidth = 1049
  Color = clBtnFace
  DefaultMonitor = dmPrimary
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clBlack
  Font.Height = -23
  Font.Name = 'Arial'
  Font.Style = []
  OldCreateOrder = False
  PopupMenu = PopupMenu
  Position = poScreenCenter
  Scaled = False
  OnActivate = FormActivate
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 120
  TextHeight = 26
  object GeneratorParsLMDGroupBox: TLMDGroupBox
    Left = 1
    Top = 100
    Width = 1046
    Height = 709
    Bevel.Mode = bmWindows
    Caption = #1043#1077#1085#1077#1088#1072#1090#1086#1088
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clBlue
    CaptionFont.Height = -23
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionFont3D.LightColor = clYellow
    CaptionFont3D.Style = tdRaised
    CaptionFont3D.Tracing = 17
    CaptionOffset = 17
    CaptionParentFont = False
    TabOrder = 0
  end
  object LoadingTestsLMDGroupBox: TLMDGroupBox
    Left = 2
    Top = 0
    Width = 428
    Height = 100
    Alignment = gcaTopCenter
    Bevel.Mode = bmWindows
    Caption = #1052#1086#1076#1091#1083#1100'   ??????'
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clBlue
    CaptionFont.Height = -20
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionFont3D.LightColor = clYellow
    CaptionFont3D.Style = tdRaised
    CaptionFont3D.Tracing = 7
    CaptionParentFont = False
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clBlue
    Font.Height = -23
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
    PopupMenu = PopupMenu
    TabOrder = 1
    object ModuleDetectionLMDShapeControl: TLMDShapeControl
      Left = 8
      Top = 24
      Width = 40
      Height = 40
      Bevel.LightColor = clWhite
      Brush.Color = clBtnFace
      Caption.Alignment = agTopLeft
      Caption.Font.Charset = RUSSIAN_CHARSET
      Caption.Font.Color = clWindowText
      Caption.Font.Height = -17
      Caption.Font.Name = 'Arial'
      Caption.Font.Style = []
      Shadow.Color = clGray
      Shape = stEllipse
      Transparent = False
      Mode = scmShapeFill
      Teeth.Height = 2
    end
    object ModuleDetectionLMDStaticText: TLMDStaticText
      Left = 51
      Top = 32
      Width = 168
      Height = 22
      Bevel.Mode = bmCustom
      Caption = #1054#1073#1085#1072#1088#1091#1078#1077#1085#1080#1077' '#1084#1086#1076#1091#1083#1103
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -18
      Font.Name = 'Times New Roman'
      Font.Style = []
      Options = []
      ParentFont = False
    end
    object VirtualSlotLabelLMDStaticText: TLMDStaticText
      Left = 59
      Top = 68
      Width = 146
      Height = 22
      Bevel.Mode = bmCustom
      Caption = #1042#1080#1088#1090#1091#1072#1083#1100#1085#1099#1081' '#1089#1083#1086#1090
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -18
      Font.Name = 'Times New Roman'
      Font.Style = []
      Options = []
      ParentFont = False
    end
    object VirtualSlotLMDStaticText: TLMDStaticText
      Left = 6
      Top = 67
      Width = 47
      Height = 27
      AutoSize = False
      Bevel.StyleInner = bvFrameLowered
      Bevel.StyleOuter = bvRaised
      Bevel.Mode = bmCustom
      Bevel.StandardStyle = lsWindowBorder
      Caption = '8'
      Color = clWhite
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -18
      Font.Name = 'Arial'
      Font.Style = []
      Options = []
      ParentFont = False
    end
    object FindModuleLMDButton: TLMDButton
      Left = 228
      Top = 34
      Width = 192
      Height = 55
      Cursor = crHandPoint
      Caption = #1054#1073#1085#1072#1088#1091#1078#1080#1090#1100' E-310'
      Enabled = False
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clNavy
      Font.Height = -20
      Font.Name = 'Courier New'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 3
      OnClick = FindModuleLMDButtonClick
      ButtonLayout.Spacing = 0
      FontFX.Style = tdRaised
      MultiLine = True
    end
  end
  object SaveIniSettingLMDGroupBox: TLMDGroupBox
    Left = 430
    Top = 0
    Width = 207
    Height = 100
    Alignment = gcaTopCenter
    Bevel.Mode = bmWindows
    Caption = #1053#1072#1089#1090#1088#1086#1081#1082#1080
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clBlue
    CaptionFont.Height = -20
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionFont3D.LightColor = clYellow
    CaptionFont3D.Style = tdRaised
    CaptionFont3D.Tracing = 7
    CaptionParentFont = False
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clWindowText
    Font.Height = -17
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
    PopupMenu = PopupMenu
    TabOrder = 2
    object AutoSaveSettingsLMDCheckBox: TLMDCheckBox
      Left = 24
      Top = 27
      Width = 157
      Height = 24
      Caption = #1040#1074#1090#1086' '#1057#1086#1093#1088#1072#1085#1077#1085#1080#1077
      Alignment.Alignment = agTopLeft
      Alignment.Spacing = 4
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clBlack
      Font.Height = -17
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnChange = AutoSaveSettingsLMDCheckBoxChange
    end
    object SaveSettingsButton: TButton
      Left = 24
      Top = 56
      Width = 160
      Height = 33
      Cursor = crHandPoint
      Caption = #1057#1086#1093#1088#1072#1085#1080#1090#1100
      Enabled = False
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -20
      Font.Name = 'Courier New'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 1
      OnClick = SaveSettingsButtonClick
    end
  end
  object PopupMenu: TPopupMenu
    Alignment = paCenter
    Left = 666
    Top = 21
    object RebootModuleMenuItem: TMenuItem
      Caption = #1055#1077#1088#1077#1079#1072#1075#1088#1091#1079#1082#1072' '#1052#1086#1076#1091#1083#1103
      Default = True
      ShortCut = 16461
      OnClick = FindModuleLMDButtonClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object SaveDefaultSettings: TMenuItem
      Caption = #1057#1086#1093#1088#1072#1085#1077#1085#1080#1077' '#1096#1090#1072#1090#1085#1099#1093' '#1085#1072#1089#1090#1088#1086#1077#1082
      ShortCut = 16452
      OnClick = SaveDefaultSettingsClick
    end
    object SaveCurrentSettings: TMenuItem
      Caption = #1057#1086#1093#1088#1072#1085#1077#1085#1080#1077' '#1090#1077#1082#1091#1097#1080#1093' '#1085#1072#1089#1090#1088#1086#1077#1082
      ShortCut = 16467
      OnClick = SaveCurrentSettingsClick
    end
    object N3: TMenuItem
      Caption = '-'
    end
    object AboutProgram: TMenuItem
      Caption = #1054' '#1087#1088#1086#1075#1088#1072#1084#1084#1077' ...'
      ShortCut = 16449
      OnClick = AboutProgramClick
    end
    object BreakLine: TMenuItem
      Caption = '-'
    end
    object ApplicationExit: TMenuItem
      Caption = #1042#1099#1093#1086#1076
      Hint = #1057#1087#1086#1082#1086#1081#1085#1099#1081' '#1074#1099#1093#1086#1076' '#1080#1079' '#1087#1088#1086#1075#1088#1072#1084#1084#1099
      ShortCut = 16465
      OnClick = ApplicationExitClick
    end
  end
end
