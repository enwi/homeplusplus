import { Component, OnInit } from '@angular/core';
import { FormGroup, FormBuilder, Validators } from '@angular/forms';
import { Router, ActivatedRoute } from '@angular/router';
import { AuthService } from '../auth/auth.service';
import { LoginService } from './login.service';

@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.css']
})
export class LoginComponent implements OnInit {
  form: FormGroup;

  constructor(private fb: FormBuilder,
    private router: Router,
    private activatedRoute: ActivatedRoute,
    private authService: AuthService,
    private loginService: LoginService) { }

  ngOnInit() {
    if (this.authService.isLoggedIn()) {
      this.redirect();
    }
    this.form = this.fb.group({
      username: ['', Validators.required],
      password: ['', Validators.required]
    });
  }

  login() {
    const val = this.form.value;
    if (val.username && val.password) {
      this.loginService.login(val.username, val.password).subscribe(() => this.redirect());
    }
  }
  private redirect() {
    let url = '/';
    if (this.activatedRoute.snapshot.paramMap.has('redirect')) {
      url = this.activatedRoute.snapshot.paramMap.get('redirect');
    }
    this.router.navigateByUrl(url);
  }
}
