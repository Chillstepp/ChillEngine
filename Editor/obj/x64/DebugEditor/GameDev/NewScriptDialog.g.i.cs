﻿#pragma checksum "..\..\..\..\GameDev\NewScriptDialog.xaml" "{8829d00f-11b8-4213-878b-770e8597ac16}" "C5DF808624296EF2022D4907287C32EAA4B18E2C5D4F671906C9ED4FA91BF4AD"
//------------------------------------------------------------------------------
// <auto-generated>
//     此代码由工具生成。
//     运行时版本:4.0.30319.42000
//
//     对此文件的更改可能会导致不正确的行为，并且如果
//     重新生成代码，这些更改将会丢失。
// </auto-generated>
//------------------------------------------------------------------------------

using Editor.GameDev;
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


namespace Editor.GameDev {
    
    
    /// <summary>
    /// NewScriptDialog
    /// </summary>
    public partial class NewScriptDialog : System.Windows.Window, System.Windows.Markup.IComponentConnector {
        
        
        #line 23 "..\..\..\..\GameDev\NewScriptDialog.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Grid BusyAnimation;
        
        #line default
        #line hidden
        
        
        #line 97 "..\..\..\..\GameDev\NewScriptDialog.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBlock MessageTextBlock;
        
        #line default
        #line hidden
        
        
        #line 102 "..\..\..\..\GameDev\NewScriptDialog.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox ScriptName;
        
        #line default
        #line hidden
        
        
        #line 106 "..\..\..\..\GameDev\NewScriptDialog.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox ScriptPath;
        
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
            System.Uri resourceLocater = new System.Uri("/Editor;component/gamedev/newscriptdialog.xaml", System.UriKind.Relative);
            
            #line 1 "..\..\..\..\GameDev\NewScriptDialog.xaml"
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
            this.BusyAnimation = ((System.Windows.Controls.Grid)(target));
            return;
            case 2:
            this.MessageTextBlock = ((System.Windows.Controls.TextBlock)(target));
            return;
            case 3:
            this.ScriptName = ((System.Windows.Controls.TextBox)(target));
            
            #line 102 "..\..\..\..\GameDev\NewScriptDialog.xaml"
            this.ScriptName.TextChanged += new System.Windows.Controls.TextChangedEventHandler(this.OnScriptName_TextBox_TextChanged);
            
            #line default
            #line hidden
            return;
            case 4:
            this.ScriptPath = ((System.Windows.Controls.TextBox)(target));
            
            #line 106 "..\..\..\..\GameDev\NewScriptDialog.xaml"
            this.ScriptPath.TextChanged += new System.Windows.Controls.TextChangedEventHandler(this.OnScriptPath_TextBox_TextChanged);
            
            #line default
            #line hidden
            return;
            case 5:
            
            #line 111 "..\..\..\..\GameDev\NewScriptDialog.xaml"
            ((System.Windows.Controls.Button)(target)).Click += new System.Windows.RoutedEventHandler(this.OnOk_Button_Click);
            
            #line default
            #line hidden
            return;
            }
            this._contentLoaded = true;
        }
    }
}

