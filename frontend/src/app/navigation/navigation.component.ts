import {BreakpointObserver, Breakpoints, BreakpointState} from '@angular/cdk/layout';
import {Component, ViewChild} from '@angular/core';
import {MatIconRegistry} from '@angular/material/icon';
import {DomSanitizer} from '@angular/platform-browser';
import {Observable} from 'rxjs';
import {map} from 'rxjs/operators';

@Component({
  selector: 'app-navigation',
  templateUrl: './navigation.component.html',
  styleUrls: ['./navigation.component.scss']
})
export class NavigationComponent {
  @ViewChild('drawer', {static: false}) m_sidenav;

  isHandset$: Observable<boolean> =
      this.breakpointObserver.observe(Breakpoints.Handset)
          .pipe(map(result => result.matches));

  constructor(
      private breakpointObserver: BreakpointObserver,
      private matIconRegistry: MatIconRegistry,
      private domSanitizer: DomSanitizer) {
    this.matIconRegistry.addSvgIcon(
        `actions`,
        this.domSanitizer.bypassSecurityTrustResourceUrl(
            '../../assets/icons/actions.svg'));
    this.matIconRegistry.addSvgIcon(
        `devices`,
        this.domSanitizer.bypassSecurityTrustResourceUrl(
            '../../assets/icons/devices.svg'));
    this.matIconRegistry.addSvgIcon(
        `home`,
        this.domSanitizer.bypassSecurityTrustResourceUrl(
            '../../assets/icons/home.svg'));
    this.matIconRegistry.addSvgIcon(
        `logout`,
        this.domSanitizer.bypassSecurityTrustResourceUrl(
            '../../assets/icons/logout.svg'));
    this.matIconRegistry.addSvgIcon(
        `rules`,
        this.domSanitizer.bypassSecurityTrustResourceUrl(
            '../../assets/icons/rules.svg'));
    this.matIconRegistry.addSvgIcon(
        `settings`,
        this.domSanitizer.bypassSecurityTrustResourceUrl(
            '../../assets/icons/settings.svg'));
    this.matIconRegistry.addSvgIcon(
        `stats`,
        this.domSanitizer.bypassSecurityTrustResourceUrl(
            '../../assets/icons/stats.svg'));
  }

  onLinkClick(): void {
    if (this.m_sidenav.mode === 'over') {
      this.m_sidenav.close();
    }
  }
}
