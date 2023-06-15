package com.lg.vps_sdk_with_config;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;

import androidx.navigation.fragment.NavHostFragment;

import com.z.s.Agent;
import com.lg.vps_sdk_with_config.databinding.FragmentFirstBinding;
import com.z.s.ISLoadCallBack;

public class FirstFragment extends Fragment {

    private FragmentFirstBinding binding;

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState
    ) {

        binding = FragmentFirstBinding.inflate(inflater, container, false);
        return binding.getRoot();

    }

    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        binding.btnInit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Agent agent=new Agent();
                agent.init(getContext(), new ISLoadCallBack() {
                    @Override
                    public void runStatus(int i) {
                        System.out.println("runStatus:"+i);
                    }
                });
            }

        });

        binding.buttonFirst.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NavHostFragment.findNavController(FirstFragment.this)
                        .navigate(R.id.action_FirstFragment_to_SecondFragment);
            }
        });
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }

}