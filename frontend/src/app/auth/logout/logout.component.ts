import { Component, OnInit, AfterViewInit, ChangeDetectorRef } from '@angular/core';
import { AuthService } from '../auth.service';
import { Router } from '@angular/router';

@Component({
  selector: 'app-logout',
  templateUrl: './logout.component.html',
  styleUrls: ['./logout.component.css']
})
export class LogoutComponent implements OnInit, AfterViewInit {

  constructor(private authService: AuthService, private router: Router, private changeDetector: ChangeDetectorRef) { }

  ngOnInit() {
  }
  ngAfterViewInit() {
    this.authService.logout();
    setTimeout(() => this.router.navigateByUrl('/login'), 1000);
  }

}
