﻿#pragma checksum "..\..\..\..\..\SubEditor\WorldEditor\GameEntityView.xaml" "{8829d00f-11b8-4213-878b-770e8597ac16}" "F2FA1FA1EF8743AE634B3883721DECBDE70A8A0F27017A4958EF8507A964D86D"
//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

using Editor.Components;
using Editor.SubEditor;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Media.TextFormatting;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Shell;


namespace Editor.SubEditor {
    
    
    /// <summary>
    /// GameEntityView
    /// </summary>
    public partial class GameEntityView : System.Windows.Controls.UserControl, System.Windows.Markup.IComponentConnector {
        
        
        #line 27 "..\..\..\..\..\SubEditor\WorldEditor\GameEntityView.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Primitives.ToggleButton AddComponent;
        
        #line default
        #line hidden
        
        private bool _contentLoaded;
        
        /// <summary>
        /// InitializeComponent
        /// </summary>
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "4.0.0.0")]
        public void InitializeComponent() {
            if (_contentLoaded) {
                return;
            }
            _contentLoaded = true;
            System.Uri resourceLocater = new System.Uri("/Editor;component/subeditor/worldeditor/gameentityview.xaml", System.UriKind.Relative);
            
            #line 1 "..\..\..\..\..\SubEditor\WorldEditor\GameEntityView.xaml"
            System.Windows.Application.LoadComponent(this, resourceLocater);
            
            #line default
            #line hidden
        }
        
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "4.0.0.0")]
        [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Never)]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Design", "CA1033:InterfaceMethodsShouldBeCallableByChildTypes")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1800:DoNotCastUnnecessarily")]
        void System.Windows.Markup.IComponentConnector.Connect(int connectionId, object target) {
            switch (connectionId)
            {
            case 1:
            this.AddComponent = ((System.Windows.Controls.Primitives.ToggleButton)(target));
            return;
            case 2:
            
            #line 48 "..\..\..\..\..\SubEditor\WorldEditor\GameEntityView.xaml"
            ((System.Windows.Controls.TextBox)(target)).GotKeyboardFocus += new System.Windows.Input.KeyboardFocusChangedEventHandler(this.OnName_TextBox_GotKeyboardFocus);
            
            #line default
            #line hidden
            
            #line 49 "..\..\..\..\..\SubEditor\WorldEditor\GameEntityView.xaml"
            ((System.Windows.Controls.TextBox)(target)).LostKeyboardFocus += new System.Windows.Input.KeyboardFocusChangedEventHandler(this.OnName_TextBox_LostKeyboardFocus);
            
            #line default
            #line hidden
            return;
            case 3:
            
            #line 52 "..\..\..\..\..\SubEditor\WorldEditor\GameEntityView.xaml"
            ((System.Windows.Controls.CheckBox)(target)).Click += new System.Windows.RoutedEventHandler(this.OnIsEnabled_CheckBox_Click);
            
            #line default
            #line hidden
            return;
            }
            this._contentLoaded = true;
        }
    }
}

