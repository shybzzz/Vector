object E310GeneratorFrame: TE310GeneratorFrame
  Left = 0
  Top = 0
  Width = 942
  Height = 592
  AutoScroll = False
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWindowText
  Font.Height = -17
  Font.Name = 'Arial'
  Font.Style = []
  ParentFont = False
  TabOrder = 0
  DesignSize = (
    942
    592)
  object MasterClockLMDGroupBox: TLMDGroupBox
    Left = 1
    Top = 2
    Width = 223
    Height = 115
    Alignment = gcaTopCenter
    Bevel.Mode = bmWindows
    Caption = #1058#1072#1082#1090#1086#1074#1072#1103' '#1095#1072#1089#1090#1086#1090#1072
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clNavy
    CaptionFont.Height = -20
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionOffset = 20
    CaptionParentFont = False
    TabOrder = 0
    object MasterClockSourceLMDRadioGroup: TLMDRadioGroup
      Left = 2
      Top = 23
      Width = 219
      Height = 56
      Bevel.Mode = bmWindows
      BtnAlignment.Alignment = agCenterLeft
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clWindowText
      CaptionFont.Height = -17
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      Items.Strings = (
        #1074#1085#1091#1090#1088#1077#1085#1085#1103#1103
        #1074#1085#1077#1096#1085#1103#1103)
      ItemOffset = 45
      MultiLine = False
      TabOrder = 0
      OnChange = MasterClockSourceLMDRadioGroupChange
    end
    object InternalMasterClockValueLMDStaticText: TLMDStaticText
      Left = 32
      Top = 83
      Width = 125
      Height = 27
      Alignment = agBottomCenter
      AutoSize = False
      Bevel.StyleInner = bvLowered
      Bevel.StyleOuter = bvRaised
      Bevel.BorderWidth = 1
      Bevel.Mode = bmCustom
      Caption = '888888888'
      Color = clWhite
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clGray
      Font.Height = -18
      Font.Name = 'Arial'
      Font.Style = []
      Options = []
      ParentFont = False
    end
    object MasterClockUnitLMDStaticText: TLMDStaticText
      Left = 159
      Top = 87
      Width = 34
      Height = 21
      Alignment = agBottomCenter
      AutoSize = False
      Bevel.Mode = bmCustom
      Caption = #1082#1043#1094
      Options = []
    end
    object ExternalMasterClockValueLMDEdit: TLMDEdit
      Left = 32
      Top = 83
      Width = 125
      Height = 27
      Bevel.Mode = bmWindows
      Caret.BlinkRate = 530
      TabOrder = 3
      OnChange = ExternalMasterClockValueLMDEditChange
      OnKeyPress = ExternalMasterClockValueLMDEditKeyPress
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clBlack
      Font.Height = -18
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      Alignment = taCenter
      CustomButtons = <>
      PasswordChar = #0
      Text = '888888888'
    end
  end
  object AnalogOutputsParsLMDGroupBox: TLMDGroupBox
    Left = 590
    Top = 2
    Width = 352
    Height = 441
    Alignment = gcaTopCenter
    Bevel.Mode = bmWindows
    Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099' '#1072#1085#1072#1083#1086#1075#1086#1074#1099#1093' '#1074#1099#1093#1086#1076#1086#1074
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clNavy
    CaptionFont.Height = -20
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionFont3D.Tracing = 1
    CaptionParentFont = False
    TabOrder = 3
    object AnalogOutputsTypeLMDRadioGroup: TLMDRadioGroup
      Left = 2
      Top = 21
      Width = 166
      Height = 90
      Bevel.Mode = bmWindows
      BtnAlignment.Alignment = agCenterLeft
      Caption = #1058#1080#1087' '#1089#1080#1075#1085#1072#1083#1072
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      Items.Strings = (
        #1090#1088#1077#1091#1075#1086#1083#1100#1085#1099#1081
        #1089#1080#1085#1091#1089#1086#1080#1076#1072#1083#1100#1085#1099#1081)
      ItemOffset = 3
      MultiLine = False
      TabOrder = 0
      OnChange = AnalogOutputsTypeLMDRadioGroupChange
    end
    object AnalogOutputsGaindBLMDGroupBox: TLMDGroupBox
      Left = 168
      Top = 30
      Width = 182
      Height = 81
      Bevel.Mode = bmWindows
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clWindowText
      CaptionFont.Height = -17
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      TabOrder = 1
      object AnalogOutputsGaindBLMDLabel: TLMDLabel
        Left = 5
        Top = 6
        Width = 171
        Height = 40
        Bevel.Mode = bmCustom
        Alignment = agCenter
        AutoSize = False
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        MultiLine = True
        Options = []
        ParentFont = False
        Caption = #1059#1089#1080#1083#1077#1085#1080#1077' '#1074#1099#1093#1086#1076#1085#1086#1075#1086' '#1090#1088#1072#1082#1090#1072' '#1075#1077#1085#1077#1088#1072#1090#1086#1088#1072
      end
      object AnalogOutputsGaindBLMDComboBox: TLMDComboBox
        Left = 43
        Top = 48
        Width = 58
        Height = 27
        DropDownCount = 14
        ItemHeight = 19
        Style = csDropDownList
        TabOrder = 0
        OnChange = AnalogOutputsGaindBLMDComboBoxChange
      end
      object AnalogOutputsGaindBLabelLMDStaticText: TLMDStaticText
        Left = 109
        Top = 51
        Width = 31
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1076#1041
        Options = []
      end
    end
    object Output10OhmParsLMDGroupBox: TLMDGroupBox
      Left = 2
      Top = 182
      Width = 348
      Height = 162
      Bevel.Mode = bmWindows
      Caption = #1042#1099#1093#1086#1076' 10 '#1054#1084
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      TabOrder = 2
      object OffsetOutput10OhmLMDGroupBox: TLMDGroupBox
        Left = 2
        Top = 63
        Width = 344
        Height = 97
        Bevel.Mode = bmWindows
        Caption = #1057#1084#1077#1097#1077#1085#1080#1077' '#1089#1080#1075#1085#1072#1083#1072
        CaptionFont.Charset = RUSSIAN_CHARSET
        CaptionFont.Color = clTeal
        CaptionFont.Height = -17
        CaptionFont.Name = 'Arial'
        CaptionFont.Style = [fsBold]
        CaptionFont3D.Tracing = 1
        CaptionParentFont = False
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clWindowText
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        object OffsetTypeOutput10OhmLMDRadioGroup: TLMDRadioGroup
          Left = 2
          Top = 18
          Width = 140
          Height = 77
          Alignment = gcaTopCenter
          Bevel.Mode = bmWindows
          BtnAlignment.Alignment = agCenterLeft
          Caption = #1058#1080#1087' '#1089#1084#1077#1097#1077#1085#1080#1103
          CaptionFont.Charset = RUSSIAN_CHARSET
          CaptionFont.Color = clMaroon
          CaptionFont.Height = -17
          CaptionFont.Name = 'Arial'
          CaptionFont.Style = []
          CaptionFont3D.Style = tdSunken
          CaptionFont3D.Tracing = 1
          CaptionParentFont = False
          Font.Charset = RUSSIAN_CHARSET
          Font.Color = clWindowText
          Font.Height = -17
          Font.Name = 'Arial'
          Font.Style = []
          Items.Strings = (
            #1074#1085#1091#1090#1088#1077#1085#1085#1077#1077
            #1074#1085#1077#1096#1085#1077#1077)
          ItemOffset = 5
          ParentFont = False
          TabOrder = 0
          OnChange = OffsetTypeOutput10OhmLMDRadioGroupChange
        end
        object OffsetValueLMDGroupBox: TLMDGroupBox
          Left = 142
          Top = 18
          Width = 200
          Height = 77
          Alignment = gcaTopCenter
          Bevel.Mode = bmWindows
          Caption = #1042#1077#1083#1080#1095#1080#1085#1072' '#1089#1084#1077#1097#1077#1085#1080#1103
          CaptionFont.Charset = RUSSIAN_CHARSET
          CaptionFont.Color = clMaroon
          CaptionFont.Height = -17
          CaptionFont.Name = 'Arial'
          CaptionFont.Style = []
          CaptionFont3D.Style = tdSunken
          CaptionFont3D.Tracing = 1
          CaptionParentFont = False
          TabOrder = 1
          object OffsetValueOutput10OhmLabelLMDStaticText: TLMDStaticText
            Left = 177
            Top = 36
            Width = 18
            Height = 21
            Alignment = agBottomCenter
            AutoSize = False
            Bevel.Mode = bmCustom
            Caption = #1042
            Options = []
          end
          object LMDStaticText5: TLMDStaticText
            Left = 5
            Top = 21
            Width = 86
            Height = 21
            Alignment = agBottomLeft
            AutoSize = False
            Bevel.Mode = bmCustom
            Caption = #1046#1077#1083#1072#1077#1084#1086#1077
            Options = []
          end
          object ActualOffsetValueOutput10OhmLMDStaticText: TLMDStaticText
            Left = 94
            Top = 47
            Width = 80
            Height = 27
            Alignment = agBottomCenter
            AutoSize = False
            Bevel.StyleInner = bvLowered
            Bevel.StyleOuter = bvRaised
            Bevel.BorderWidth = 1
            Bevel.Mode = bmCustom
            Caption = '888888'
            Color = clWhite
            Font.Charset = RUSSIAN_CHARSET
            Font.Color = clGreen
            Font.Height = -18
            Font.Name = 'Arial'
            Font.Style = []
            Options = []
            ParentFont = False
          end
          object ActualOffsetValueOutput10OhmLabelLMDStaticText: TLMDStaticText
            Left = 5
            Top = 50
            Width = 85
            Height = 21
            Alignment = agBottomLeft
            AutoSize = False
            Bevel.Mode = bmCustom
            Caption = #1056#1077#1072#1083#1100#1085#1086#1077
            Options = []
          end
          object DesiredOffsetValueOutput10OhmLMDEdit: TLMDEdit
            Left = 94
            Top = 19
            Width = 80
            Height = 27
            Bevel.Mode = bmWindows
            Caret.BlinkRate = 530
            TabOrder = 0
            OnChange = DesiredOffsetValueOutput10OhmLMDEditChange
            OnKeyPress = DesiredOffsetValueOutput10OhmLMDEditKeyPress
            Font.Charset = RUSSIAN_CHARSET
            Font.Color = clWindowText
            Font.Height = -18
            Font.Name = 'Arial'
            Font.Style = []
            ParentFont = False
            Alignment = taCenter
            CustomButtons = <>
            PasswordChar = #0
            Text = '888888'
          end
        end
      end
      object Output10OhmAmplitudeLMDStaticText: TLMDStaticText
        Left = 18
        Top = 26
        Width = 160
        Height = 21
        Alignment = agBottomLeft
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1040#1084#1087#1083#1080#1090#1091#1076#1072' '#1089#1080#1075#1085#1072#1083#1072
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object ApmlitudeOutput10OhmIndBLMDStaticText: TLMDStaticText
        Left = 187
        Top = 37
        Width = 97
        Height = 27
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.StyleInner = bvLowered
        Bevel.StyleOuter = bvRaised
        Bevel.Mode = bmCustom
        Caption = '888888888'
        Color = clWhite
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object ApmlitudeOutput10OhmIndBLabelLMDStaticText: TLMDStaticText
        Left = 288
        Top = 40
        Width = 30
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1076#1041
        Options = []
      end
      object ApmlitudeOutput10OhmInVLMDStaticText: TLMDStaticText
        Left = 187
        Top = 10
        Width = 97
        Height = 27
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.StyleInner = bvLowered
        Bevel.StyleOuter = bvRaised
        Bevel.Mode = bmCustom
        Caption = '888888888'
        Color = clWhite
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object ApmlitudeOutput10OhmInVLabelLMDStaticText: TLMDStaticText
        Left = 288
        Top = 13
        Width = 20
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1042
        Options = []
      end
    end
    object Output50OhmParsLMDGroupBox: TLMDGroupBox
      Left = 2
      Top = 112
      Width = 348
      Height = 68
      Bevel.Mode = bmWindows
      Caption = #1042#1099#1093#1086#1076' 50 '#1054#1084
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      TabOrder = 3
      object Output50OhmAmplitudeLMDStaticText: TLMDStaticText
        Left = 18
        Top = 25
        Width = 160
        Height = 21
        Alignment = agBottomLeft
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1040#1084#1087#1083#1080#1090#1091#1076#1072' '#1089#1080#1075#1085#1072#1083#1072
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object ApmlitudeOutput50OhmIndBLMDStaticText: TLMDStaticText
        Left = 187
        Top = 37
        Width = 97
        Height = 27
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.StyleInner = bvLowered
        Bevel.StyleOuter = bvRaised
        Bevel.Mode = bmCustom
        Caption = '888888888'
        Color = clWhite
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object LMDStaticText2: TLMDStaticText
        Left = 287
        Top = 40
        Width = 30
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1076#1041
        Options = []
      end
      object ApmlitudeOutput50OhmInVLMDStaticText: TLMDStaticText
        Left = 187
        Top = 10
        Width = 97
        Height = 27
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.StyleInner = bvLowered
        Bevel.StyleOuter = bvRaised
        Bevel.Mode = bmCustom
        Caption = '888888888'
        Color = clWhite
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object LMDStaticText4: TLMDStaticText
        Left = 287
        Top = 13
        Width = 20
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1042
        Options = []
      end
    end
  end
  object FrequencyParsLMDGroupBox: TLMDGroupBox
    Left = 224
    Top = 2
    Width = 366
    Height = 441
    Alignment = gcaTopCenter
    Bevel.Mode = bmWindows
    Caption = #1063#1072#1089#1090#1086#1090#1085#1099#1077' '#1087#1072#1088#1072#1084#1077#1090#1088#1099
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clNavy
    CaptionFont.Height = -20
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionFont3D.Tracing = 5
    CaptionParentFont = False
    TabOrder = 2
    object FinalFrequencyValueLMDStaticText: TLMDStaticText
      Left = 178
      Top = 219
      Width = 125
      Height = 27
      Alignment = agBottomCenter
      AutoSize = False
      Bevel.StyleInner = bvLowered
      Bevel.StyleOuter = bvRaised
      Bevel.Mode = bmCustom
      Caption = '888888888'
      Color = clWhite
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clGreen
      Font.Height = -18
      Font.Name = 'Arial'
      Font.Style = []
      Options = []
      ParentFont = False
    end
    object FinalFrequencyUnitLMDStaticText: TLMDStaticText
      Left = 306
      Top = 222
      Width = 34
      Height = 21
      Alignment = agBottomCenter
      AutoSize = False
      Bevel.Mode = bmCustom
      Caption = #1082#1043#1094
      Options = []
    end
    object FinalFrequencyLabelLMDStaticText: TLMDStaticText
      Left = 7
      Top = 222
      Width = 169
      Height = 21
      Alignment = agBottomLeft
      AutoSize = False
      Bevel.Mode = bmCustom
      Caption = #1050#1086#1085#1077#1095#1085#1072#1103' '#1095#1072#1089#1090#1086#1090#1072
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clPurple
      Font.Height = -17
      Font.Name = 'Arial'
      Font.Style = []
      Options = []
      ParentFont = False
    end
    object NumberOfIncrementsLabelLMDStaticText: TLMDStaticText
      Left = 7
      Top = 193
      Width = 169
      Height = 21
      Alignment = agBottomLeft
      AutoSize = False
      Bevel.Mode = bmCustom
      Caption = #1050#1086#1083'-'#1074#1086' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1081
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clPurple
      Font.Height = -17
      Font.Name = 'Arial'
      Font.Style = []
      Options = []
      ParentFont = False
    end
    object NumberOfIncrementsValueLMDEdit: TLMDEdit
      Left = 178
      Top = 191
      Width = 125
      Height = 27
      Bevel.Mode = bmWindows
      Caret.BlinkRate = 530
      TabOrder = 2
      OnChange = NumberOfIncrementsValueLMDEditChange
      OnKeyPress = NumberOfIncrementsValueLMDEditKeyPress
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -18
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      Alignment = taCenter
      CustomButtons = <>
      PasswordChar = #0
      Text = '888888888'
    end
    object IncrementIntervalParsLMDGroupBox: TLMDGroupBox
      Left = 2
      Top = 248
      Width = 362
      Height = 191
      Bevel.Mode = bmWindows
      Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099' '#1080#1085#1090#1077#1088#1074#1072#1083#1072' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1103
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      TabOrder = 3
      object BaseIntervalsNumberLabelLMDLabel: TLMDLabel
        Left = 4
        Top = 115
        Width = 187
        Height = 35
        Bevel.Mode = bmCustom
        Alignment = agCenterLeft
        AutoSize = False
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -15
        Font.Name = 'Arial'
        Font.Style = []
        MultiLine = True
        Options = []
        ParentFont = False
        Caption = #1050#1086#1083#1080#1095#1077#1089#1090#1074#1086' '#1073#1072#1079#1086#1074#1099#1093' '#1080#1085#1090#1077#1088#1074#1072#1083#1086#1074' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1103
      end
      object DurationLabelLMDLabel: TLMDLabel
        Left = 4
        Top = 153
        Width = 180
        Height = 35
        Bevel.Mode = bmCustom
        Alignment = agCenterLeft
        AutoSize = False
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -15
        Font.Name = 'Arial'
        Font.Style = []
        MultiLine = True
        Options = []
        ParentFont = False
        Caption = #1054#1073#1097#1072#1103' '#1076#1083#1080#1090#1077#1083#1100#1085#1086#1089#1090#1100' '#1086#1076#1085#1086#1075#1086' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1103
      end
      object BaseIntervalsNumberLMDEdit: TLMDEdit
        Left = 191
        Top = 120
        Width = 100
        Height = 27
        Bevel.Mode = bmWindows
        Caret.BlinkRate = 530
        TabOrder = 1
        OnChange = BaseIntervalsNumberLMDEditChange
        OnKeyPress = BaseIntervalsNumberLMDEditKeyPress
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clWindowText
        Font.Height = -18
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        Alignment = taCenter
        CustomButtons = <>
        PasswordChar = #0
        Text = 'CCCCC'
      end
      object DurationLMDStaticText: TLMDStaticText
        Left = 191
        Top = 157
        Width = 135
        Height = 27
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.StyleInner = bvLowered
        Bevel.StyleOuter = bvRaised
        Bevel.Mode = bmCustom
        Caption = 'CCCCC'
        Color = clWhite
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -18
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object DurationUnitLMDStaticText: TLMDStaticText
        Left = 328
        Top = 160
        Width = 29
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1084#1089
        Options = []
      end
      object BaseIntervalParsLMDGroupBox: TLMDGroupBox
        Left = 3
        Top = 20
        Width = 357
        Height = 93
        Bevel.Mode = bmWindows
        Caption = #1041#1072#1079#1086#1074#1099#1081' '#1080#1085#1090#1077#1088#1074#1072#1083' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1103
        CaptionFont.Charset = RUSSIAN_CHARSET
        CaptionFont.Color = clTeal
        CaptionFont.Height = -17
        CaptionFont.Name = 'Arial'
        CaptionFont.Style = [fsBold]
        CaptionFont3D.Tracing = 1
        CaptionParentFont = False
        TabOrder = 0
        object BaseIntervalMultiplierLMDRadioGroup: TLMDRadioGroup
          Left = 238
          Top = 18
          Width = 117
          Height = 73
          Alignment = gcaTopCenter
          Bevel.Mode = bmWindows
          BtnAlignment.Alignment = agCenterLeft
          Caption = #1052#1085#1086#1078#1080#1090#1077#1083#1100
          CaptionFont.Charset = RUSSIAN_CHARSET
          CaptionFont.Color = clMaroon
          CaptionFont.Height = -17
          CaptionFont.Name = 'Arial'
          CaptionFont.Style = []
          CaptionFont3D.Style = tdSunken
          CaptionFont3D.Tracing = 1
          CaptionOffset = 10
          CaptionParentFont = False
          Columns = 2
          Font.Charset = RUSSIAN_CHARSET
          Font.Color = clBlack
          Font.Height = -15
          Font.Name = 'Arial'
          Font.Style = []
          Items.Strings = (
            'x 1'
            'x 5'
            'x 100'
            'x 500')
          ItemOffset = 2
          ParentFont = False
          TabOrder = 1
          OnChange = BaseIntervalMultiplierLMDRadioGroupChange
        end
        object BaseIntervalTypeLMDRadioGroup: TLMDRadioGroup
          Left = 2
          Top = 18
          Width = 236
          Height = 73
          Bevel.Mode = bmWindows
          BtnAlignment.Alignment = agCenterLeft
          Caption = #1041#1072#1079#1086#1074#1099#1081' '#1080#1085#1090#1077#1088#1074#1072#1083' '#1082#1088#1072#1090#1077#1085
          CaptionFont.Charset = RUSSIAN_CHARSET
          CaptionFont.Color = clMaroon
          CaptionFont.Height = -17
          CaptionFont.Name = 'Arial'
          CaptionFont.Style = []
          CaptionFont3D.Style = tdSunken
          CaptionFont3D.Tracing = 1
          CaptionOffset = 6
          CaptionParentFont = False
          Font.Charset = RUSSIAN_CHARSET
          Font.Color = clBlack
          Font.Height = -15
          Font.Name = 'Arial'
          Font.Style = []
          Items.Strings = (
            #1087#1077#1088#1080#1086#1076#1091' '#1074#1099#1093#1086#1076#1085#1086#1075#1086' '#1089#1080#1075#1085#1072#1083#1072
            #1087#1077#1088#1080#1086#1076#1091' '#1090#1072#1082#1090#1086#1074#1086#1081' '#1095#1072#1089#1090#1086#1090#1099)
          ItemOffset = 3
          ParentFont = False
          TabOrder = 0
          TabStop = True
          OnChange = BaseIntervalTypeLMDRadioGroupChange
        end
      end
    end
    object StartFrequencyLMDGroupBox: TLMDGroupBox
      Left = 2
      Top = 20
      Width = 362
      Height = 83
      Bevel.Mode = bmWindows
      Caption = #1053#1072#1095#1072#1083#1100#1085#1072#1103' '#1095#1072#1089#1090#1086#1090#1072
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      TabOrder = 0
      object DesiredStartFrequencyValueLMDEdit: TLMDEdit
        Left = 145
        Top = 24
        Width = 125
        Height = 27
        Bevel.Mode = bmWindows
        Caret.BlinkRate = 530
        TabOrder = 0
        OnChange = DesiredStartFrequencyValueLMDEditChange
        OnKeyPress = DesiredStartFrequencyValueLMDEditKeyPress
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clWindowText
        Font.Height = -18
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        Alignment = taCenter
        CustomButtons = <>
        PasswordChar = #0
        Text = '888888888'
      end
      object StartFrequencyUnitLMDStaticText: TLMDStaticText
        Left = 275
        Top = 40
        Width = 34
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1082#1043#1094
        Options = []
      end
      object ActualStartFrequencyValueLMDStaticText: TLMDStaticText
        Left = 145
        Top = 52
        Width = 125
        Height = 27
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.StyleInner = bvLowered
        Bevel.StyleOuter = bvRaised
        Bevel.BorderWidth = 1
        Bevel.Mode = bmCustom
        Caption = '888888888'
        Color = clWhite
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clGreen
        Font.Height = -18
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
      object DesiredStartFrequencyLabelLMDStaticText: TLMDStaticText
        Left = 53
        Top = 27
        Width = 88
        Height = 21
        Alignment = agBottomLeft
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1046#1077#1083#1072#1077#1084#1072#1103
        Options = []
      end
      object ActualStartFrequencyLMDStaticText: TLMDStaticText
        Left = 53
        Top = 55
        Width = 88
        Height = 21
        Alignment = agBottomLeft
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1056#1077#1072#1083#1100#1085#1072#1103
        Options = []
      end
    end
    object FrequencyIncrementLMDGroupBox: TLMDGroupBox
      Left = 2
      Top = 104
      Width = 362
      Height = 83
      Bevel.Mode = bmWindows
      Caption = #1064#1072#1075' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1103' '#1087#1086' '#1095#1072#1089#1090#1086#1090#1077
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      TabOrder = 1
      object DesiredFrequencyIncrementLabelLMDStaticText: TLMDStaticText
        Left = 53
        Top = 28
        Width = 88
        Height = 21
        Alignment = agBottomLeft
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1046#1077#1083#1072#1077#1084#1099#1081
        Options = []
      end
      object DesiredFrequencyIncrementValueLMDEdit: TLMDEdit
        Left = 145
        Top = 24
        Width = 125
        Height = 27
        Bevel.Mode = bmWindows
        Caret.BlinkRate = 530
        TabOrder = 1
        OnChange = DesiredFrequencyIncrementValueLMDEditChange
        OnKeyPress = DesiredFrequencyIncrementValueLMDEditKeyPress
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clWindowText
        Font.Height = -18
        Font.Name = 'Arial'
        Font.Style = []
        ParentFont = False
        Alignment = taCenter
        CustomButtons = <>
        PasswordChar = #0
        Text = '888888888'
      end
      object FrequencyIncrementsUnitLMDStaticText: TLMDStaticText
        Left = 275
        Top = 40
        Width = 34
        Height = 21
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1082#1043#1094
        Options = []
      end
      object ActualFrequencyIncrementLabeLMDStaticText: TLMDStaticText
        Left = 53
        Top = 55
        Width = 88
        Height = 21
        Alignment = agBottomLeft
        AutoSize = False
        Bevel.Mode = bmCustom
        Caption = #1056#1077#1072#1083#1100#1085#1099#1081
        Options = []
      end
      object ActualFrequencyIncrementLMDStaticText: TLMDStaticText
        Left = 145
        Top = 52
        Width = 125
        Height = 27
        Alignment = agBottomCenter
        AutoSize = False
        Bevel.StyleInner = bvLowered
        Bevel.StyleOuter = bvRaised
        Bevel.BorderWidth = 1
        Bevel.Mode = bmCustom
        Caption = '888888888'
        Color = clWhite
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clGreen
        Font.Height = -18
        Font.Name = 'Arial'
        Font.Style = []
        Options = []
        ParentFont = False
      end
    end
  end
  object CyclicAutoScanLMDRadioGroup: TLMDRadioGroup
    Left = 1
    Top = 118
    Width = 223
    Height = 104
    Alignment = gcaTopCenter
    Bevel.Mode = bmWindows
    BtnAlignment.Alignment = agCenterLeft
    Caption = #1062#1080#1082#1083'. '#1072#1074#1090#1086#1089#1082#1072#1085#1080#1088#1086#1074#1072#1085#1080#1077
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clNavy
    CaptionFont.Height = -18
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionOffset = 7
    CaptionParentFont = False
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clBlack
    Font.Height = -18
    Font.Name = 'Arial'
    Font.Style = []
    Items.Strings = (
      #1053#1077#1090
      #39#1055#1080#1083#1072#39
      #39#1058#1088#1077#1091#1075#1086#1083#1100#1085#1080#1082#39)
    ItemOffset = 40
    ParentFont = False
    TabOrder = 1
    TabStop = True
    OnChange = CyclicAutoScanLMDRadioGroupChange
  end
  object DigitalLinesParsLMDGroupBox: TLMDGroupBox
    Left = 1
    Top = 446
    Width = 942
    Height = 146
    Alignment = gcaTopCenter
    Bevel.Mode = bmWindows
    Caption = #1062#1080#1092#1088#1086#1074#1086#1081' '#1074#1099#1093#1086#1076'  '#1080'  '#1083#1080#1085#1080#1080' '#1091#1087#1088#1072#1074#1083#1077#1085#1080#1103
    CaptionFont.Charset = RUSSIAN_CHARSET
    CaptionFont.Color = clNavy
    CaptionFont.Height = -20
    CaptionFont.Name = 'Arial'
    CaptionFont.Style = []
    CaptionFont3D.Tracing = 7
    CaptionParentFont = False
    TabOrder = 4
    object CtrlLineTypeLMDRadioGroup: TLMDRadioGroup
      Left = 276
      Top = 25
      Width = 191
      Height = 71
      Bevel.Mode = bmWindows
      BtnAlignment.Alignment = agCenterLeft
      Caption = #1051#1080#1085#1080#1103' '#39'CTRL'#39
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      Items.Strings = (
        #1074#1085#1091#1090#1088#1077#1085#1085#1103#1103
        #1074#1085#1077#1096#1085#1103#1103)
      ItemOffset = 25
      TabOrder = 0
      OnChange = CtrlLineTypeLMDRadioGroupChange
    end
    object InterrupLineTypeLMDRadioGroup: TLMDRadioGroup
      Left = 467
      Top = 25
      Width = 191
      Height = 71
      Bevel.Mode = bmWindows
      BtnAlignment.Alignment = agCenterLeft
      Caption = #1051#1080#1085#1080#1103' '#39'INTERRUP'#39
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      Items.Strings = (
        #1074#1085#1091#1090#1088#1077#1085#1085#1103#1103
        #1074#1085#1077#1096#1085#1103#1103)
      ItemOffset = 25
      TabOrder = 1
      OnChange = InterrupLineTypeLMDRadioGroupChange
    end
    object SyncoutLineParsLMDGroupBox: TLMDGroupBox
      Left = 658
      Top = 25
      Width = 282
      Height = 119
      Bevel.Mode = bmWindows
      Caption = #1051#1080#1085#1080#1103' '#39'SYNCOUT'#39
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionParentFont = False
      TabOrder = 2
      object EnaSyncoutLMDCheckBox: TLMDCheckBox
        Left = 23
        Top = 23
        Width = 236
        Height = 22
        Caption = #1056#1072#1079#1088#1077#1096#1077#1085#1080#1077' '#1089#1080#1085#1093#1088#1086#1074#1099#1093#1086#1076#1072
        Alignment.Alignment = agTopLeft
        Alignment.Spacing = 4
        TabOrder = 0
        OnChange = EnaSyncoutLMDCheckBoxChange
      end
      object SyncoutTypeLMDRadioGroup: TLMDRadioGroup
        Left = 2
        Top = 46
        Width = 278
        Height = 71
        Bevel.Mode = bmWindows
        BtnAlignment.Alignment = agCenterLeft
        Caption = #1058#1080#1087' '#1092#1086#1088#1084#1080#1088#1086#1074#1072#1085#1080#1103
        CaptionFont.Charset = RUSSIAN_CHARSET
        CaptionFont.Color = clTeal
        CaptionFont.Height = -17
        CaptionFont.Name = 'Arial'
        CaptionFont.Style = [fsBold]
        CaptionFont3D.Tracing = 1
        CaptionParentFont = False
        Items.Strings = (
          #1087#1088#1080' '#1082#1072#1078#1076#1086#1084' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1080
          #1074' '#1082#1086#1085#1094#1077' '#1089#1082#1072#1085#1080#1088#1086#1074#1072#1085#1080#1103)
        ItemOffset = 30
        TabOrder = 1
        OnChange = SyncoutTypeLMDRadioGroupChange
      end
    end
    object IncrementTypeLMDRadioGroup: TLMDRadioGroup
      Left = 2
      Top = 25
      Width = 274
      Height = 71
      Bevel.Mode = bmWindows
      BtnAlignment.Alignment = agCenterLeft
      Caption = #1059#1087#1088#1072#1074#1083#1077#1085#1080#1077' '#1087#1088#1080#1088#1072#1097#1077#1085#1080#1103#1084#1080
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clPurple
      CaptionFont.Height = -18
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      CaptionFont3D.Tracing = 1
      CaptionOffset = 7
      CaptionParentFont = False
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -17
      Font.Name = 'Arial'
      Font.Style = []
      Items.Strings = (
        #1072#1074#1090#1086#1084#1072#1090#1080#1095#1077#1089#1082#1086#1077
        #1083#1080#1085#1080#1077#1081' '#39'CTRL'#39)
      ItemOffset = 45
      ParentFont = False
      TabOrder = 3
      OnChange = IncrementTypeLMDRadioGroupChange
    end
    object MiscellaneousLMDGroupBox: TLMDGroupBox
      Left = 2
      Top = 96
      Width = 365
      Height = 48
      Bevel.Mode = bmWindows
      CaptionFont.Charset = RUSSIAN_CHARSET
      CaptionFont.Color = clWindowText
      CaptionFont.Height = -17
      CaptionFont.Name = 'Arial'
      CaptionFont.Style = []
      TabOrder = 4
      object EnaSquareWaveOutputLMDCheckBox: TLMDCheckBox
        Left = 8
        Top = 14
        Width = 353
        Height = 24
        Caption = #1062#1080#1092#1088#1086#1074#1086#1081' '#1074#1099#1093#1086#1076' "'#1052#1077#1072#1085#1076#1088'"'
        Alignment.Alignment = agTopLeft
        Alignment.Spacing = 11
        AutoSize = True
        Font.Charset = RUSSIAN_CHARSET
        Font.Color = clBlack
        Font.Height = -17
        Font.Name = 'Arial'
        Font.Style = []
        Font3D.Tracing = 5
        ParentFont = False
        TabOrder = 0
        OnChange = EnaSquareWaveOutputLMDCheckBoxChange
      end
    end
  end
  object ControlGeneratorLMDButton: TLMDButton
    Left = 2
    Top = 290
    Width = 216
    Height = 86
    Cursor = crHandPoint
    Anchors = [akTop]
    Caption = #1047#1072#1087#1091#1089#1082' '#1075#1077#1085#1077#1088#1072#1090#1086#1088#1072
    Font.Charset = RUSSIAN_CHARSET
    Font.Color = clGreen
    Font.Height = -27
    Font.Name = 'Courier New'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 5
    OnClick = ControlGeneratorLMDButtonClick
    ButtonLayout.Spacing = 0
    ButtonStyle = ubsWin31
    FontFX.Style = tdRaised
    FontFX.Tracing = 4
    MultiLine = True
  end
end
